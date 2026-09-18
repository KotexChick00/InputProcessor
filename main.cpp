// Use project IWindow and GLFWWindow to create a black 800x600 window titled "Hello Window"

#include <memory>
#include <glad/glad.h>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Window/FrameLimiter.hpp>
#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>
#include <Renderer/Opengl/OpenglRenderer.hpp>
#include <UI/Imgui/ImguiUIRenderer.hpp>
#include <UI/Imgui/ImguiUIWindowVisitor.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
// do not include <imgui_impl_opengl3_loader.h> because glad is already included

using namespace InputProcessor::Logger::SpdLog;

using namespace InputProcessor::Window;
using namespace InputProcessor::Window::GLFW;
using namespace InputProcessor::Renderer::Opengl;
using namespace InputProcessor::Renderer::Resource::Opengl;
using namespace InputProcessor::Renderer;
using namespace InputProcessor::Renderer::Resource;
using namespace InputProcessor::UI::Imgui;
using namespace InputProcessor::UI;

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

const unsigned int WINDOW_WIDTH = 1000;
const unsigned int WINDOW_HEIGHT = 1000;

int main() {
    Logger::SetEngineImplementation(std::make_shared<SpdLogLoggerAdapter>("IP_ENGINE"));
    std::unique_ptr<GLFWWindow> window = std::make_unique<GLFWWindow>();

    WindowConfiguration config{ WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Window" };
    window->Init(config);
    FrameLimiter frameLimiter(config.TargetFPS);

    IRenderer* renderer = OpenglRenderer::GetInstance();
    RendererConfiguration rendererConfig = renderer->GetConfig();
    rendererConfig.ClearBufferColor.Red = 0.2f;
    rendererConfig.ClearBufferColor.Green = 0.3f;
    rendererConfig.ClearBufferColor.Blue = 0.3f;
    rendererConfig.ClearBufferColor.Alpha = 1.0f;
    rendererConfig.ViewPortOptions.Width = WINDOW_WIDTH;
    rendererConfig.ViewPortOptions.Height = WINDOW_HEIGHT;

    renderer->Config(rendererConfig);
    
    IShader* shader = OpenglShader::FromSource(vertexShaderSource, fragmentShaderSource);
    IVertexBuffer* vertexBuffer = OpenglVertexBuffer::Create();
    vertexBuffer->SetData(0, vertices, sizeof(vertices), 3, 3 * sizeof(float));

    IIndexBuffer* indexBuffer = OpenglIndexBuffer::Create();
    indexBuffer->SetData(indices, sizeof(indices));


	// Init ImGui
    ImguiWindowContextInitVisitor* contextVisitor = new ImguiWindowContextInitVisitor();
    ImguiWindowRenderVisitor* renderVisitor = new ImguiWindowRenderVisitor();
    ImguiWindowShutdownVisitor* shutdownVisitor = new ImguiWindowShutdownVisitor();
    IUIRenderer* uiRenderer = new ImguiUIRenderer(contextVisitor, renderVisitor, shutdownVisitor);
    uiRenderer->Init(window.get());

    while (!window->CheckShouldClose()) {
        frameLimiter.BeginFrame();

        // Render scene (triangle)
        renderer->GetRendererCommand()->ClearBuffers(ClearBufferMasks::Color);
        shader->Use();
        vertexBuffer->Bind();
        indexBuffer->Bind();
        renderer->GetRendererCommand()->DrawIndex(RenderMode::Triangles, 6);

        uiRenderer->Render();

        window->SwapBuffers();
        window->PollEvents();
        frameLimiter.EndFrame();
    }

    uiRenderer->Free();
    delete uiRenderer;
    delete contextVisitor;
    delete renderVisitor;
    delete shutdownVisitor;

    window->Close();
    OpenglRenderer::Free();
    return 0;
}
