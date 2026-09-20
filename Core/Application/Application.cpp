#include <Window/WindowPlatformFactory.hpp>
#include <Logger/LoggerFactory.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Application/Application.hpp>

namespace CoreEngine {
	Application::Application(ApplicationConfiguration& configuration) : mConfig(configuration) { }

	bool Application::Init() {
		SetupLogger();
		SetupWindow();
		SetupRenderer();
		return mWindow != nullptr;
	}

	void Application::Run() {
		if (!Init()) CORE_ASSERT("Failed to Init Application");
		OnInitClient();

		while (!mWindow->CheckShouldClose()) {
			BeginLoop();
			OnLoopClient();
			EndLoop();
		}

		OnShutdownClient();
		Shutdown();
	}

	void Application::BeginLoop() {
		Time::Time::Update();
		mWindow->PollEvents();
	}

	void Application::EndLoop() {
		mWindow->SwapBuffers();
	}

	void Application::Shutdown() {
		mWindow->Close();
	}

	void Application::SetupWindow() {
		if (mConfig.WindowPlatformSpec == WindowPlatformSpec::Injection) {
			mWindow.reset(mConfig.InjectedWindow);
		}
		else {
			Window::IWindow* window = nullptr;
			switch (mConfig.WindowPlatformSpec) {
			case WindowPlatformSpec::GLFW:
				window = Window::WindowPlatformFactory::Create(Window::WindowPlatformSpec::GLFW);
				break;
			case WindowPlatformSpec::SDL:
				window = Window::WindowPlatformFactory::Create(Window::WindowPlatformSpec::SDL);
				break;
			default:
				CORE_ASSERT("Currently Window platform spec not found");
			}
			mWindow.reset(window);
		}

		Window::WindowConfiguration windowConfig;
		windowConfig.Width = mConfig.Width;
		windowConfig.Height = mConfig.Height;
		windowConfig.Title = mConfig.Title;

		mWindow->Init(windowConfig);
	}

	void Application::SetupLogger() {
		Logger::Logger::SetClientImplementation(Logger::LoggerFactory::Create(Logger::LoggerSpec::SpdLog, "CORE_ENGINE"));
		Logger::Logger::SetEngineImplementation(Logger::LoggerFactory::Create(Logger::LoggerSpec::SpdLog, "CLIENT"));
	}

	void Application::SetupRenderer() {
		if (mConfig.RenderAPI == RenderAPI::Injection) {
			mRenderer.reset(mConfig.InjectedRenderer);
		}
		else {
			Renderer::RendererSpec spec;
			switch (mConfig.RenderAPI) {
			case RenderAPI::Opengl: 
				spec = Renderer::RendererSpec::Opengl;
				break;
			case RenderAPI::DirectX:
				spec = Renderer::RendererSpec::DirectX;
				break;
			case RenderAPI::Vulkan:
				spec = Renderer::RendererSpec::Vulkan;
				break;
			default:
				CORE_ASSERT("Can't find the renderer API match");
				break;
			}

			mRenderer.reset(Renderer::RendererFactory::Create(spec));
		}
	}
}