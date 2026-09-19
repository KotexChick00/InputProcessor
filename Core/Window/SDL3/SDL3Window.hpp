#pragma once
#include <Window/IWindow.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <SDL3/SDL.h>
#include <functional>
#include <vector>


namespace InputProcessor::Window::SDL3 {
	class SDL3Window : public InputProcessor::Window::IWindow {
	public:
		void Init(const WindowConfiguration& config) override;
		void PollEvents() override;
		void SwapBuffers() override;
		bool CheckShouldClose() override;
		void Close() override;
		void Accept(IWindowVisitor* visitor) override;
		SDL_Window* GetNativeWindow();
		using NativeEventHook = std::function<void(const SDL_Event&)>;
		void AddNativeEventHook(NativeEventHook hook) { mEventHooks.push_back(std::move(hook)); }
		SDL_GLContext GetGLContext() const { return mGLContext; }
	private:
		SDL_Window* mWindow{ nullptr };
		SDL_GLContext mGLContext{ nullptr };
		bool mShouldClose{ false };
		std::vector<NativeEventHook> mEventHooks;
	};
}

