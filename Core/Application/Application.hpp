#pragma once
#include <Application/ApplicationEventContext.hpp>
#include <Window/IWindow.hpp>
#include <Renderer/IRenderer.hpp>
#include <Input/InputState.hpp>
#include <Time/Time.hpp>
#include <Event/EventDispatcher.hpp>
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

		unsigned int MaxFPS = 60;

		WindowPlatformSpec WindowPlatformSpec = WindowPlatformSpec::GLFW;
		Window::IWindow* InjectedWindow = nullptr;			// If WindowPlatformSpec is Injection

		RenderAPI RenderAPI = RenderAPI::Opengl;
		Renderer::IRenderer* InjectedRenderer = nullptr;	// If RenderAPI is Injection
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
		CORE_FORCE_INLINE Time::Time* GetTime() const { return Time::Time::GetInstance(); }
		CORE_FORCE_INLINE Input::InputState GetInput() const { return mWindow->GetInput(); }
		CORE_FORCE_INLINE Event::EventDispatcher* GetEventDispatcher() const { return (Event::EventDispatcher*)&mEventDispatcher; }

	protected:
		Application(ApplicationConfiguration& configuration);

	private:
		bool Init();
		void BeginLoop();
		void EndLoop();
		void Shutdown();

		void SetupLogger();
		void SetupWindow();
		void SetupEvents();
		void SetupRenderer();

	private:
		ApplicationConfiguration mConfig;
		Unique<Window::IWindow> mWindow = nullptr;
		Unique<Renderer::IRenderer> mRenderer = nullptr;

		Event::EventDispatcher mEventDispatcher;

	private:
		static KeyboardKey ToApplicationKeyboardKey(Window::WindowKeyboardKey key);
		static KeyboardKeyState ToApplicationKeyboardKeyState(Window::WindowKeyboardKeyState keyState);
		static MouseButton ToApplicationMouseButton(Window::WindowMouseButton mouseButton);
		static MouseButtonState ToApplicationMouseButtonState(Window::WindowMouseButtonState mouseButtonState);
	};
}