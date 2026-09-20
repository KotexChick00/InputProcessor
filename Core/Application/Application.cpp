#include <Window/WindowPlatformFactory.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Logger/LoggerFactory.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Application/Application.hpp>
#include <Input/GLFW/GLFWKeyBoardInput.hpp>
#include <Input/GLFW/GLFWMouseInput.hpp>

namespace CoreEngine {
	Application::Application(ApplicationConfiguration& configuration) : mConfig(configuration) { }

	bool Application::Init() {
		SetupLogger();
		SetupWindow();
		SetupInput();
		SetupEvents();
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
		Time::Time* time = GetTime();
		float minDeltaTime = 1.f / mConfig.MaxFPS;
		float prevFrameTime = time->GetTimeNow();

		while (mWindow->GetCurrentSeconds() - prevFrameTime < minDeltaTime);
		float currentTime = mWindow->GetCurrentSeconds();
		time->SetDeltaTime(currentTime - prevFrameTime);
		time->SetTimeNow(currentTime);

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
		Logger::Logger::SetClientImplementation(Logger::LoggerFactory::Create(Logger::LoggerSpec::SpdLog, "CLIENT"));
		Logger::Logger::SetEngineImplementation(Logger::LoggerFactory::Create(Logger::LoggerSpec::SpdLog, "CORE_ENGINE"));
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

	void Application::SetupInput() {
		if (mConfig.WindowPlatformSpec == WindowPlatformSpec::GLFW) {
			Window::GLFW::GLFWWindow* window = dynamic_cast<Window::GLFW::GLFWWindow*>(mWindow.get());
			CORE_ASSERT(window != nullptr && "Window is not glfw");
			mKeyboardInput.reset(new Input::GLFW::GLFWKeyBoardInput(window->GetNativeWindow()));
			mMouseInput.reset(new Input::GLFW::GLFWMouseInput(window->GetNativeWindow()));
		}
		else {
			CORE_ASSERT("Currently window platform is not supported");
		}
	}

	void Application::SetupEvents() {
		mWindow->OnWindowReiszeEventCallback([this](CoreEngine::Window::WindowResizeEventContext ec) {
			WindowResizeEventContext evc(ec.Width, ec.Height);
			this->mEventDispatcher.Notify(evc);
			});
	}
}