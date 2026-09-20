#include <iostream>
#include <Application/Application.hpp>

class DemoApp : public CoreEngine::Application {
public:
    DemoApp(CoreEngine::ApplicationConfiguration& config) : Application(config) { }

protected:
    void OnInitClient() override {
        CoreEngine::Renderer::IRenderer* renderer = GetRenderer();
        CoreEngine::Renderer::RendererConfiguration config = renderer->GetConfig();
        config.ClearBufferColor.Red = 1.0f;
        renderer->Config(config);
    }

    void OnLoopClient() override {
        GetRenderer()->GetRendererCommand()->ClearBuffers(CoreEngine::Renderer::ClearBufferMasks::Color);
        IP_CLIENT_TRACE("Delta Time: {}", GetTime()->GetDeltaTime());
    }

    void OnShutdownClient() override {
        std::cout << "CLIENT SHUTDOWN" << std::endl;
    }
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
