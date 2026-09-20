#include <glad/glad.h>
#include <Window/IWindowVisitor.hpp>
#include <Window/SDL3/SDL3Window.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Window::SDL3 {
	using namespace CoreEngine::Logger;
	
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
			switch (e.type) {
			case SDL_EVENT_QUIT:
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				mShouldClose = true; break;
			case SDL_EVENT_MOUSE_MOTION:      HandleMouseMotionEvent(e.motion); break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:   HandleMouseButtonEvent(e.button); break;
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:            HandleKeyboardEvent(e.key); break;
			case SDL_EVENT_MOUSE_WHEEL:       HandleMouseScrollEvent(e.wheel); break;
			default: break;
			}
		}
	}

	void SDL3Window::OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> cb) { mMouseMoveEventCallback = std::move(cb); }
	void SDL3Window::OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> cb) { mMouseButtonEventCallback = std::move(cb); }
	void SDL3Window::OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> cb) { mKeyboardEventCallback = std::move(cb); }
	void SDL3Window::OnMouseScrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> cb) { mMouseScrollEventCallback = std::move(cb); }


	float SDL3Window::GetCurrentSeconds() {
		return static_cast<float>(SDL_GetTicks()) / 1000.0f;   // ms kể từ SDL_Init, giống glfwGetTime
	}


	void SDL3Window::HandleMouseMotionEvent(const SDL_MouseMotionEvent& e) {
		if (!mMouseMoveEventCallback) return;
		WindowMouseMoveEventContext ctx = { e.x, e.y };
		mMouseMoveEventCallback(ctx);
	}

	void SDL3Window::HandleMouseButtonEvent(const SDL_MouseButtonEvent& e) {
		if (!mMouseButtonEventCallback) return;
		WindowMouseButtonEventContext ctx = { _ToWindowMouseButton(e.button), _ToWindowMouseButtonState(e.down) };
		mMouseButtonEventCallback(ctx);
	}

	void SDL3Window::HandleKeyboardEvent(const SDL_KeyboardEvent& e) {
		if (!mKeyboardEventCallback) return;
		WindowKeyboardKeyEventContext ctx = { _ToWindowKeyboardKey(e.scancode), _ToWindowKeyboardState(e.down, e.repeat) };
		mKeyboardEventCallback(ctx);
	}

	void SDL3Window::HandleMouseScrollEvent(const SDL_MouseWheelEvent& e) {
		if (!mMouseScrollEventCallback) return;
		WindowMouseScrollEventContext ctx = { _ToWindowMouseScrollDirection(e.x, e.y), e.x, e.y };
		mMouseScrollEventCallback(ctx);
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