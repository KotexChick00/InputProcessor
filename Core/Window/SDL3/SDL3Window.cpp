#include <glad/glad.h>
#include <Window/IWindowVisitor.hpp>
#include <Window/SDL3/SDL3Window.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Window::SDL3 {
	using namespace InputProcessor::Logger;

	void SDL3Window::Init(const WindowConfiguration& config) {
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			IP_ENGINE_CRITICAL("Failed to initialize SDL: {}", SDL_GetError());
			return;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

		// Không có SDL_WINDOW_RESIZABLE => tương đương GLFW_RESIZABLE = FALSE
		mWindow = SDL_CreateWindow(config.Title.c_str(), config.Width, config.Height, SDL_WINDOW_OPENGL);
		if (!mWindow) {
			IP_ENGINE_CRITICAL("Failed to create SDL window: '{}' ({}x{}) - {}", config.Title, config.Width, config.Height, SDL_GetError());
			SDL_Quit();
			return;
		}

		mGLContext = SDL_GL_CreateContext(mWindow);
		if (!mGLContext) {
			IP_ENGINE_CRITICAL("Failed to create OpenGL context: {}", SDL_GetError());
			Close();
			return;
		}
		SDL_GL_MakeCurrent(mWindow, mGLContext);

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
			IP_ENGINE_CRITICAL("Failed to initialize OpenGL loader (GLAD).");
			Close();
			return;
		}

		IP_ENGINE_TRACE("Window initialized successfully: '{}' ({}x{})", config.Title, config.Width, config.Height);
	}

	void SDL3Window::PollEvents() {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			for (auto& hook : mEventHooks) hook(e);   // ImGui, debug, ...

			if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				mShouldClose = true;
			}
		}
	}

	SDL_Window* SDL3Window::GetNativeWindow() {
		return mWindow;
	}

	void SDL3Window::SwapBuffers() {
		if (mWindow) {
			SDL_GL_SwapWindow(mWindow);
		}
	}

	bool SDL3Window::CheckShouldClose() {
		return mWindow ? mShouldClose : true;
	}

	void SDL3Window::Close() {
		if (mGLContext) {
			SDL_GL_DestroyContext(mGLContext);
			mGLContext = nullptr;
		}
		if (mWindow) {
			SDL_DestroyWindow(mWindow);
			mWindow = nullptr;
			SDL_Quit();
			IP_ENGINE_TRACE("Window closed and SDL terminated.");
		}
	}

	void SDL3Window::Accept(IWindowVisitor* visitor) {
		visitor->VisitSdl3Window(this);
	}
}