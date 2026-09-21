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

        CoreEngine::Event::EventDispatcher* eventDispatcher = GetEventDispatcher();
        eventDispatcher->AddEventListener<CoreEngine::WindowResizeEventContext>([](const CoreEngine::WindowResizeEventContext& eventContext) -> bool {
            IP_CLIENT_TRACE("({}, {})", eventContext.GetWidth(), eventContext.GetHeight());
            return true;
            });
    }

    void OnLoopClient() override {
        GetRenderer()->GetRendererCommand()->ClearBuffers(CoreEngine::Renderer::ClearBufferMasks::Color);
        auto input = GetInput();
        IP_CLIENT_TRACE("({}, {})", input.MouseInput->GetDeltaX(), input.MouseInput->GetDeltaY());
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
