#pragma once
#include <Window/IWindow.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <SDL3/SDL.h>
#include <functional>
#include <vector>


namespace CoreEngine::Window::SDL3 {
	class SDL3Window : public CoreEngine::Window::IWindow {
	public:
		void Init(const WindowConfiguration& config) override;
		void PollEvents() override;
		void SwapBuffers() override;
		bool CheckShouldClose() override;
		void Close() override;

		void OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) override;
		void OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) override;
		void OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) override;
		void OnMouseScrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) override;

		float GetCurrentSeconds() override;

		void Accept(IWindowVisitor* visitor) override;

		SDL_Window* GetNativeWindow();

		void HandleMouseMotionEvent(const SDL_MouseMotionEvent& event);
		void HandleMouseButtonEvent(const SDL_MouseButtonEvent& event);
		void HandleKeyboardEvent(const SDL_KeyboardEvent& event);
		void HandleMouseScrollEvent(const SDL_MouseWheelEvent& event);


		using NativeEventHook = std::function<void(const SDL_Event&)>;
		void AddNativeEventHook(NativeEventHook hook) { mEventHooks.push_back(std::move(hook)); }
		SDL_GLContext GetGLContext() const { return mGLContext; }

	private:
		SDL_Window* mWindow{ nullptr };
		SDL_GLContext mGLContext{ nullptr };
		bool mShouldClose{ false };
		std::vector<NativeEventHook> mEventHooks;
		std::function<void(const WindowMouseMoveEventContext&)> mMouseMoveEventCallback = nullptr;
		std::function<void(const WindowMouseButtonEventContext&)> mMouseButtonEventCallback = nullptr;
		std::function<void(const WindowKeyboardKeyEventContext&)> mKeyboardEventCallback = nullptr;
		std::function<void(const WindowMouseScrollEventContext&)> mMouseScrollEventCallback = nullptr;

	private:
		static WindowMouseButton _ToWindowMouseButton(Uint8 button);
		static WindowMouseButtonState _ToWindowMouseButtonState(bool pressed);
		static WindowKeyboardKey _ToWindowKeyboardKey(SDL_Scancode scancode);
		static WindowKeyboardKeyState _ToWindowKeyboardState(bool pressed, bool repeat);
		static WindowMouseScrollDirection _ToWindowMouseScrollDirection(float xOffset, float yOffset);

	};
}

