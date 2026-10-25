#include <Application/Application.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <Model/Importers/ModelImporter/GltfModelFormatHandler.hpp>
#include <Model/Importers/ModelImporter/ModelImporter.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpBoundingBox.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

namespace {
namespace Orbit = Domain::Camera::OrbitCamera;
constexpr auto vertexSource = R"(#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;
layout(location=3) in vec3 aColor;
uniform mat4 uViewProjection;
out vec3 normal;
out vec2 texCoords;
out vec3 vertexColor;
void main() {
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
    normal = aNormal;
    texCoords = aTexCoords;
    vertexColor = aColor;
})";
constexpr auto fragmentSource = R"(#version 330 core
in vec3 normal;
in vec2 texCoords;
in vec3 vertexColor;
struct Material { sampler2D diffuse[1]; };
uniform Material material;
uniform bool uHasDiffuseTexture;
uniform vec4 uBaseColor;
out vec4 color;
void main() {
    vec4 base = uBaseColor * vec4(vertexColor, 1.0);
    if (uHasDiffuseTexture) base *= texture(material.diffuse[0], texCoords);
    float light = 0.35 + 0.65 * abs(dot(normalize(normal), normalize(vec3(1, 2, 3))));
    color = vec4(base.rgb * light, base.a);
})";

class ModelViewer : public CoreEngine::Application {
public:
    ModelViewer(CoreEngine::ApplicationConfiguration& config, std::string path, int frameLimit)
        : Application(config), mPath(std::move(path)), mFrameLimit(frameLimit) {}

    void OnInitClient() override {
        auto* renderer = GetRenderer();
        auto config = renderer->GetConfig();
        config.DepthOptions.Enabled = true;
        config.DepthOptions.Operation = CoreEngine::Renderer::DepthOperation::ReadAndWrite;
        config.ClearBufferColor = {0.08f, 0.09f, 0.12f, 1.0f};
        renderer->Config(config);
        mShader = renderer->GetResourceManager()->CreateShaderFromSources(vertexSource, fragmentSource);
        if (!mShader || !mShader->GetShaderId()) throw std::runtime_error("Failed to create model shader");
        Model::GltfModelFormatHandler handler(renderer->GetResourceManager());
        Model::ModelImporter importer(&handler);
        mModel = importer.Import(mPath.c_str());
        if (!mModel || mModel->GetMeshes().empty()) throw std::runtime_error("Failed to read GLTF/GLB model: " + mPath);
        for (const auto& mesh : mModel->GetMeshes())
            for (const auto& vertex : mesh.Vertices) mBounds.expand(vertex.Position);
        if (!mBounds.isValid()) throw std::runtime_error("Model contains no vertices");
        FitCamera();
        mController = std::make_unique<Orbit::OrbitCameraController>(mCamera, GetInput());
        GetEventDispatcher()->AddEventListener<CoreEngine::WindowResizeEventContext>([this](const auto& event) {
            mCamera.SetViewportSize(event.GetWidth(), event.GetHeight());
            return false;
        });
    }

    void OnLoopClient() override {
        auto input = GetInput();
        if (input.KeyboardInput->CheckIsPressed(CoreEngine::Input::KeyboardKey::Escape)) RequestClose();
        if (input.KeyboardInput->CheckIsJustPressed(CoreEngine::Input::KeyboardKey::R)) FitCamera();
        mController->Update(GetTime()->GetDeltaTime());
        auto* renderer = GetRenderer();
        renderer->GetRendererCommand()->ClearBuffers(CoreEngine::Renderer::ClearBufferMasks::Color | CoreEngine::Renderer::ClearBufferMasks::Depth);
        mShader->Use();
        auto viewProjection = mCamera.GetProjectionMatrix() * mCamera.GetViewMatrix();
        mShader->SetUniformMatrix4fv("uViewProjection", glm::value_ptr(viewProjection));
        mModel->Render(renderer, mShader);
        if (mFrameLimit > 0 && ++mFrames >= mFrameLimit) RequestClose();
    }

    void OnShutdownClient() override {
        mController.reset();
        mModel.reset(); // GPU buffers must be released before the GL context.
        mShader = nullptr; // Owned by the renderer's resource manager.
    }

private:
    void FitCamera() {
        auto viewport = GetRenderer()->GetConfig().ViewPortOptions;
        float aspect = static_cast<float>(viewport.Width) / std::max(1u, viewport.Height);
        mScale = std::max(mBounds.radius(), 0.001f);
        auto fit = Model::FitCameraToBox(mBounds, 45.0f, aspect);
        mCamera = Orbit::OrbitCamera(fit.Target, 1.0f);
        mCamera.SetZoomLimits(mScale * 0.01f, mScale * 100.0f);
        mCamera.SetDistance(glm::length(fit.Position - fit.Target));
        mCamera.SetPerspective(glm::radians(45.0f), mScale * 0.001f, mScale * 110.0f);
        mCamera.SetViewportSize(viewport.Width, viewport.Height);
        mCamera.CaptureAsInitial();
    }

    std::string mPath;
    int mFrameLimit = 0, mFrames = 0;
    float mScale = 1.0f;
    Model::AssimpBoundingBox mBounds;
    Orbit::OrbitCamera mCamera{glm::vec3(0), 5.0f};
    std::unique_ptr<Orbit::OrbitCameraController> mController;
    std::unique_ptr<Model::RenderModel> mModel;
    CoreEngine::Renderer::IShader* mShader = nullptr;
};
}

#ifndef INPUTPROCESSOR_SAMPLE_MODEL
#define INPUTPROCESSOR_SAMPLE_MODEL "tests/assets/textured.gltf"
#endif

int main(int argc, char** argv) {
    bool useSample = argc == 1 || (argc == 3 && std::string_view(argv[1]) == "--frames");
    if (!useSample && argc != 2 && argc != 4) {
        std::cerr << "Usage: InputProcessor [model.gltf|model.glb] [--frames N]\n"
                  << "Drag left mouse: orbit; scroll/PageUp/PageDown: zoom; arrows: orbit; WASD: pan; R: fit; Esc: close\n";
        return 1;
    }
    try {
        int frameLimit = 0;
        if (argc == 3 || argc == 4) {
            int flagIndex = useSample ? 1 : 2;
            if (std::string_view(argv[flagIndex]) != "--frames") throw std::invalid_argument("Expected --frames N");
            std::string count(argv[flagIndex + 1]);
            size_t end = 0;
            frameLimit = std::stoi(count, &end);
            if (end != count.size() || frameLimit <= 0) throw std::invalid_argument("Frame count must be positive");
        }
        CoreEngine::ApplicationConfiguration config{1000, 700, "GLTF Viewer"};
        ModelViewer viewer(config, useSample ? INPUTPROCESSOR_SAMPLE_MODEL : argv[1], frameLimit);
        viewer.Run();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
