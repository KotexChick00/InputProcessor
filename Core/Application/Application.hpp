#pragma once
#include <Window/IWindow.hpp>
#include <Renderer/IRenderer.hpp>
#include <Time/Time.hpp>
#include <pch.h>

namespace CoreEngine {
	enum class CORE_API WindowPlatformSpec {
		GLFW,
		SDL,
		Injection // This will tell the application that the configuration will use the Injected window 
	};

	enum class CORE_API RenderAPI {
		Opengl,
		DirectX,
		Vulkan,
		Injection // This will tell the application that the configuration will use the Injected RenderAPI
	};

	struct CORE_API ApplicationConfiguration {
		unsigned int Width;
		unsigned int Height;
		const char* Title;

		WindowPlatformSpec WindowPlatformSpec = WindowPlatformSpec::GLFW;
		Window::IWindow* InjectedWindow = nullptr; // If WindowPlatformSpec is Injection

		RenderAPI RenderAPI = RenderAPI::Opengl;
		Renderer::IRenderer* InjectedRenderer = nullptr; // If RenderAPI is Injection
	};

	class CORE_API Application {
	public:
		virtual ~Application() = default;

		void Run();

		// Client
		virtual void OnInitClient() { }
		virtual void OnLoopClient() { }
		virtual void OnShutdownClient() { }

		// Client API
		CORE_FORCE_INLINE Renderer::IRenderer* GetRenderer() const { return mRenderer.get(); }

	protected:
		Application(ApplicationConfiguration& configuration);

	private:
		bool Init();
		void BeginLoop();
		void EndLoop();
		void Shutdown();

		void SetupLogger();
		void SetupWindow();
		void SetupRenderer();	

	private:
		ApplicationConfiguration mConfig;
		Unique<Window::IWindow> mWindow = nullptr;
		Unique<Renderer::IRenderer> mRenderer = nullptr;
	};
}