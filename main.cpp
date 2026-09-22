#include <iostream>
#include <Application/Application.hpp>

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

const char* vertexSrc = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main() {\n"
"   gl_Position = vec4(aPos, 1.0f);\n"
"}\n";

const char* fragmentSource = "#version 330 core\n"
"out vec4 color;\n"
"void main() {\n"
"   color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n";

class DemoApp : public CoreEngine::Application {
public:
    DemoApp(CoreEngine::ApplicationConfiguration& config) : Application(config) { }

protected:
    void OnInitClient() override {
        CoreEngine::Renderer::IRenderer* renderer = GetRenderer();
        CoreEngine::Renderer::RendererConfiguration config = renderer->GetConfig();
        config.ClearBufferColor.Red = 1.0f;
        renderer->Config(config);

        shader = renderer->GetResourceManager()->CreateShaderFromSources(vertexSrc, fragmentSource);
        vertexBuffer = renderer->GetResourceManager()->CreateVertexBuffer();
        vertexBuffer->SetData(0, vertices, sizeof(vertices), 3, 3 * sizeof(float));
    }

    void OnLoopClient() override {
        GetRenderer()->GetRendererCommand()->ClearBuffers(CoreEngine::Renderer::ClearBufferMasks::Color);
        auto input = GetInput();
        shader->Use();
        vertexBuffer->Bind();
        GetRenderer()->GetRendererCommand()->Draw(CoreEngine::Renderer::RenderMode::Triangles, 3);
    }

    void OnShutdownClient() override {
        std::cout << "CLIENT SHUTDOWN" << std::endl;
    }

private:
    CoreEngine::Renderer::IShader* shader = nullptr;
    CoreEngine::Renderer::IVertexBuffer* vertexBuffer = nullptr;
};

int main() {
    CoreEngine::ApplicationConfiguration config;
    config.Width = 1000;
    config.Height = 1000;
    config.Title = "Hello World!";
    config.WindowPlatformSpec = CoreEngine::WindowPlatformSpec::GLFW;
    config.RenderAPI = CoreEngine::RenderAPI::Opengl;

    CoreEngine::Application* application = new DemoApp(config);
    application->Run();
    delete application;
}
