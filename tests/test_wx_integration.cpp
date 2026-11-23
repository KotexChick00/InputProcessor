// Force `assert()` to remain active even when the build system defines
// NDEBUG (Release presets, third-party toolchains). Without this guard the
// entire test suite silently degrades to a no-op because every check is an
// `assert(...)` call.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <glad/glad.h>
#include <Application/Application.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <Window/Wx/WxWindow.hpp>
#include <cassert>
#include <filesystem>
#include <iostream>

class TrackingWindow : public CoreEngine::WxWindow {
public:
    int closeCalls = 0;
    int openFileRequests = 0;
    int dialogCalls = 0;
    CoreEngine::WindowFileDialogOptions lastDialogOptions;
    std::string cannedOpenPath; // empty: dialog reports "user cancelled"

    void Close() override { ++closeCalls; WxWindow::Close(); }

    // Must not show a real modal dialog (not automatable): return the canned
    // path instead, so the client flow around the dialog stays testable.
    bool ShowOpenFileDialog(const CoreEngine::WindowFileDialogOptions& options, std::string& outPath) override {
        ++dialogCalls;
        lastDialogOptions = options;
        outPath = cannedOpenPath;
        return !cannedOpenPath.empty();
    }
};

class InputApplication : public CoreEngine::Application {
public:
    InputApplication(CoreEngine::ApplicationConfiguration& config, TrackingWindow* window, bool requestClose)
        : Application(config), window(window), requestClose(requestClose) {}
    int frames = 0;
    bool stopped = false;

    void OnInitClient() override {
        frame = static_cast<wxFrame*>(wxTopLevelWindows.GetFirst()->GetData());
        canvas = dynamic_cast<wxGLCanvas*>(frame->GetChildren().GetFirst()->GetData());
        assert(canvas && GLAD_GL_VERSION_3_3);
        inputState = &GetInput();
        controller = std::make_unique<Domain::Camera::OrbitCamera::OrbitCameraController>(camera, *inputState);

        // Keep the teammate's framebuffer factory usable alongside embedded texture uploads.
        auto* framebuffer = GetRenderer()->GetResourceManager()->CreateColorFrameBuffer(64, 32);
        assert(framebuffer && framebuffer->GetFrameBufferId());
        auto size = framebuffer->GetViewPortSize();
        assert(size.Width == 64 && size.Height == 32);
        float clearColor[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        framebuffer->Bind();
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        unsigned char framebufferPixel[4] = {};
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, framebufferPixel);
        assert(framebufferPixel[0] == 255 && framebufferPixel[1] == 0);
        framebuffer->Unbind();
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        delete framebuffer;

        // Uploading an index buffer must preserve the currently selected mesh's VAO state.
        GLuint vao = 0, previousEbo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &previousEbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, previousEbo);
        auto* indexBuffer = GetRenderer()->GetResourceManager()->CreateIndexBuffer();
        unsigned int indices[] = {0, 2, 1};
        indexBuffer->SetData(indices, sizeof(indices));
        GLint attachedEbo = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &attachedEbo);
        assert(static_cast<GLuint>(attachedEbo) == previousEbo);
        glBindBuffer(GL_COPY_READ_BUFFER, indexBuffer->GetIndexBufferId());
        unsigned int uploadedIndices[3] = {};
        glGetBufferSubData(GL_COPY_READ_BUFFER, 0, sizeof(uploadedIndices), uploadedIndices);
        assert(uploadedIndices[0] == 0 && uploadedIndices[1] == 2 && uploadedIndices[2] == 1);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &previousEbo);
        delete indexBuffer;
        assert(glGetError() == GL_NO_ERROR);

        Model::AssimpModelImporter importer(GetRenderer()->GetResourceManager());
        model = importer.Import(TEST_ASSET_DIR "/embedded.glb");
        assert(model && glGetError() == GL_NO_ERROR); // Decode and upload the actual embedded PNG.
        shader = GetRenderer()->GetResourceManager()->CreateShaderFromSources(R"(#version 330 core
            layout(location=0) in vec3 position;
            layout(location=2) in vec2 uv;
            out vec2 texCoords;
            void main() { gl_Position = vec4((position.xy - vec2(3,4)) * 0.8, 0, 1); texCoords = uv; }
        )", R"(#version 330 core
            in vec2 texCoords;
            struct Material { sampler2D diffuse[1]; };
            uniform Material material;
            uniform bool uHasDiffuseTexture;
            uniform vec4 uBaseColor;
            out vec4 color;
            void main() { color = uBaseColor; if (uHasDiffuseTexture) color *= texture(material.diffuse[0], texCoords); }
        )");
        assert(shader && shader->GetShaderId());
        assert(GetRenderer()->GetResourceManager()->GetShader(shader->GetShaderId()) == shader);
        GetEventDispatcher()->AddEventListener<CoreEngine::WindowResizeEventContext>([this](const auto& event) {
            width = event.GetWidth();
            height = event.GetHeight();
            return false;
        });

        // The window owns a File menu with an "Open Model..." item bound to Ctrl+O.
        wxMenuBar* menuBar = frame->GetMenuBar();
        assert(menuBar);
        wxMenuItem* openItem = menuBar->FindItem(wxID_OPEN);
        assert(openItem && openItem->GetItemLabelText() == "Open Model...");
        const wxAcceleratorEntry* accelerator = openItem->GetAccel();
        assert(accelerator && accelerator->GetFlags() == wxACCEL_CTRL && accelerator->GetKeyCode() == 'O');

        // Firing the menu command before a callback is registered must be a no-op.
        wxCommandEvent early(wxEVT_MENU, wxID_OPEN);
        frame->GetEventHandler()->ProcessEvent(early);
        assert(window->openFileRequests == 0);

        window->OnOpenFileRequestCallback([this]() { ++openRequests; });
    }

    void OnLoopClient() override {
        assert(inputState == &GetInput());
        auto input = *inputState;
        using CoreEngine::MouseButton;
        using CoreEngine::KeyboardKey;
        if (frames == 0) {
            wxKeyEvent key(wxEVT_KEY_DOWN);
            key.m_keyCode = 'R';
            canvas->GetEventHandler()->ProcessEvent(key);
            wxMouseEvent hover(wxEVT_MOTION);
            hover.SetPosition({10, 10});
            canvas->GetEventHandler()->ProcessEvent(hover);
            for (auto button : {MouseButton::ButtonRight, MouseButton::ButtonMiddle}) {
                wxPoint pressPosition = button == MouseButton::ButtonRight ? wxPoint(15, 17) : wxPoint(25, 29);
                wxMouseEvent move(wxEVT_MOTION);
                move.SetPosition(pressPosition);
                canvas->GetEventHandler()->ProcessEvent(move);
                assert(input.MouseInput->GetDeltaX() != 0 || input.MouseInput->GetDeltaY() != 0);
                wxMouseEvent down(button == MouseButton::ButtonRight ? wxEVT_RIGHT_DOWN : wxEVT_MIDDLE_DOWN);
                down.SetPosition(pressPosition);
                canvas->GetEventHandler()->ProcessEvent(down);
                assert(input.MouseInput->GetDeltaX() == 0 && input.MouseInput->GetDeltaY() == 0);
                wxMouseEvent up(button == MouseButton::ButtonRight ? wxEVT_RIGHT_UP : wxEVT_MIDDLE_UP);
                up.SetPosition(pressPosition);
                canvas->GetEventHandler()->ProcessEvent(up);
                canvas->GetEventHandler()->ProcessEvent(hover);
            }
            wxMouseEvent down(wxEVT_LEFT_DOWN);
            down.SetPosition({10, 10});
            canvas->GetEventHandler()->ProcessEvent(down);
            assert(input.MouseInput->GetDeltaX() == 0 && input.MouseInput->GetDeltaY() == 0);
            wxMouseEvent move(wxEVT_MOTION);
            move.SetPosition({13, 14});
            canvas->GetEventHandler()->ProcessEvent(move);
            move.SetPosition({18, 20});
            canvas->GetEventHandler()->ProcessEvent(move);
            wxMouseEvent wheel(wxEVT_MOUSEWHEEL);
            wheel.m_wheelDelta = 120;
            wheel.m_wheelRotation = 120;
            canvas->GetEventHandler()->ProcessEvent(wheel);
            wheel.m_wheelRotation = 240;
            canvas->GetEventHandler()->ProcessEvent(wheel);
            assert(input.KeyboardInput->CheckIsJustPressed(KeyboardKey::R));
            assert(input.MouseInput->CheckIsJustPressed(MouseButton::ButtonLeft));
            assert(input.MouseInput->GetDeltaX() == 8 && input.MouseInput->GetDeltaY() == 10);
            assert(input.MouseInput->GetScrollY() == 3);
            key.m_keyCode = WXK_RIGHT;
            canvas->GetEventHandler()->ProcessEvent(key);
            controller->Update(0);
            controller->Update(0.1f);
            assert(camera.GetAzimuth() != 0);
        } else if (frames == 1) {
            assert(!input.KeyboardInput->CheckIsJustPressed(KeyboardKey::R));
            assert(input.KeyboardInput->CheckIsPressed(KeyboardKey::R));
            assert(input.MouseInput->CheckIsPressed(MouseButton::ButtonLeft));
            assert(!input.MouseInput->CheckIsJustPressed(MouseButton::ButtonLeft));
            assert(input.MouseInput->GetDeltaX() == 0 && input.MouseInput->GetScrollY() == 0);
            wxFocusEvent lost(wxEVT_KILL_FOCUS);
            canvas->GetEventHandler()->ProcessEvent(lost);
            assert(!input.KeyboardInput->CheckIsPressed(KeyboardKey::R));
            assert(!input.MouseInput->CheckIsPressed(MouseButton::ButtonLeft));
            auto position = camera.GetPosition();
            controller->Update(0.1f);
            assert(glm::length(camera.GetPosition() - position) < 0.001f);
            wxSizeEvent resize({640, 360});
            canvas->GetEventHandler()->ProcessEvent(resize);
            auto viewport = GetRenderer()->GetConfig().ViewPortOptions;
            assert(width == viewport.Width && height == viewport.Height && width > 0 && height > 0);
        } else {
            wxKeyEvent key(wxEVT_KEY_DOWN);
            key.m_keyCode = WXK_F24;
            canvas->GetEventHandler()->ProcessEvent(key);
            assert(input.KeyboardInput->CheckIsPressed(KeyboardKey::F24));

            // wxGTK có thể gửi lại cùng một key-down (cùng mã phím + timestamp), kể cả sau key-up:
            // canvas phải bỏ bản lặp để cú bấm nhanh được tính đúng một lần và phím không bị kẹt.
            wxKeyEvent tapDown(wxEVT_KEY_DOWN);
            tapDown.m_keyCode = 'Q';
            tapDown.SetTimestamp(123456);
            wxKeyEvent tapUp(wxEVT_KEY_UP);
            tapUp.m_keyCode = 'Q';
            tapUp.SetTimestamp(123460);
            canvas->GetEventHandler()->ProcessEvent(tapDown);
            canvas->GetEventHandler()->ProcessEvent(tapUp);
            canvas->GetEventHandler()->ProcessEvent(tapDown); // bản lặp đến sau key-up
            assert(input.KeyboardInput->CheckIsJustPressed(KeyboardKey::Q));
            assert(!input.KeyboardInput->CheckIsPressed(KeyboardKey::Q));
            // Lần bấm mới (timestamp khác) vẫn được nhận.
            wxKeyEvent nextDown(wxEVT_KEY_DOWN);
            nextDown.m_keyCode = 'Q';
            nextDown.SetTimestamp(123999);
            canvas->GetEventHandler()->ProcessEvent(nextDown);
            assert(input.KeyboardInput->CheckIsPressed(KeyboardKey::Q));
            // Menu command reaches the registered callback (same path as a real click).
            assert(openRequests == 0);
            wxCommandEvent openCmd(wxEVT_MENU, wxID_OPEN);
            frame->GetEventHandler()->ProcessEvent(openCmd);
            assert(openRequests == 1);

            // The dialog API round-trips options and returns the chosen path...
            CoreEngine::WindowFileDialogOptions options;
            options.Title = "Unit test dialog";
            options.Filters = "GLTF (*.gltf)|*.gltf";
            options.StartDirectory = TEST_ASSET_DIR;
            std::string chosenPath;
            window->cannedOpenPath = TEST_ASSET_DIR "/embedded.glb";
            assert(window->ShowOpenFileDialog(options, chosenPath));
            assert(chosenPath == window->cannedOpenPath);
            assert(window->lastDialogOptions.Title == options.Title);
            assert(window->lastDialogOptions.Filters == options.Filters);
            assert(window->lastDialogOptions.StartDirectory == options.StartDirectory);

            // ...and cancelling (empty canned path) returns false without a path.
            window->cannedOpenPath.clear();
            assert(!window->ShowOpenFileDialog(options, chosenPath));
            assert(chosenPath.empty());
            assert(window->dialogCalls == 2);

            if (requestClose) RequestClose();
            else frame->Close(); // Native close event must request shutdown without recursion.
        }
        GetRenderer()->GetRendererCommand()->ClearBuffers(CoreEngine::ClearBufferMasks::Color);
        model->Render(GetRenderer(), shader);
        unsigned char pixel[4] = {};
        auto viewport = GetRenderer()->GetConfig().ViewPortOptions;
        glReadPixels(viewport.Width / 4, viewport.Height / 4, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        assert(pixel[0] + pixel[1] + pixel[2] > 20); // A textured triangle must actually reach the framebuffer.
        assert(glGetError() == GL_NO_ERROR);
        ++frames;
        assert(frames <= 3);
    }

    void OnShutdownClient() override {
        // A programmatic close must leave the window/context alive until GPU cleanup.
        if (requestClose) assert(window->closeCalls == 0);
        assert(glGetString(GL_VERSION));
        controller.reset();
        model.reset();
        stopped = true;
    }

private:
    TrackingWindow* window;
    bool requestClose;
    int openRequests = 0;
    wxFrame* frame = nullptr;
    wxGLCanvas* canvas = nullptr;
    unsigned int width = 0, height = 0;
    std::unique_ptr<Model::RenderModel> model;
    CoreEngine::IShader* shader = nullptr;
    const CoreEngine::InputState* inputState = nullptr;
    Domain::Camera::OrbitCamera::OrbitCamera camera{glm::vec3(0), 5};
    std::unique_ptr<Domain::Camera::OrbitCamera::OrbitCameraController> controller;
};

int main() {
    // Recreating the application exercises wxApp, renderer and GL resource lifetimes.
    for (int run = 0; run < 2; ++run) {
        CoreEngine::ApplicationConfiguration config{320, 240, "wx integration check"};
        config.MaxFPS = 100;
        auto* window = new TrackingWindow();
        config.WindowPlatformSpec = CoreEngine::WindowPlatformSpec::Injection;
        config.InjectedWindow = window;
        InputApplication app(config, window, run == 1);
        app.Run();
        assert(app.frames == 3 && app.stopped);
    }
    std::cout << "wx application/input/resize/embedded texture/close integration PASS\n";
}
