#include <glad/glad.h>
#include <Window/IWindowVisitor.hpp>
#include <Window/SDL3/SDL3Window.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Window::SDL3 {
	using namespace CoreEngine::Logger;
	
	bool SDL3Window::TryInit(const WindowConfiguration& config) {
		if (mWindow) return true;   // tránh khởi tạo hai lần
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			IP_ENGINE_CRITICAL("Failed to initialize SDL: {}", SDL_GetError());
			return false;
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
			return false;
		}

		mGLContext = SDL_GL_CreateContext(mWindow);
		if (!mGLContext) {
			IP_ENGINE_CRITICAL("Failed to create OpenGL context: {}", SDL_GetError());
			Close();
			return false;
		}
		SDL_GL_MakeCurrent(mWindow, mGLContext);

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
			IP_ENGINE_CRITICAL("Failed to initialize OpenGL loader (GLAD).");
			Close();
			return false;
		}

		IP_ENGINE_TRACE("Window initialized successfully: '{}' ({}x{})", config.Title, config.Width, config.Height);
		return true;
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


	double SDL3Window::GetCurrentSeconds() {
		return SDL_GetTicksNS() * 1e-9;
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

	WindowMouseButton SDL3Window::_ToWindowMouseButton(Uint8 button) {
		switch (button) {
		case SDL_BUTTON_LEFT: return WindowMouseButton::Button1;
		case SDL_BUTTON_RIGHT: return WindowMouseButton::Button2;
		case SDL_BUTTON_MIDDLE: return WindowMouseButton::Button3;
		case SDL_BUTTON_X1: return WindowMouseButton::Button4;
		case SDL_BUTTON_X2: return WindowMouseButton::Button5;
		default: return WindowMouseButton::ButtonLast; // Invalid or unknown button
		}
	}

	WindowMouseButtonState SDL3Window::_ToWindowMouseButtonState(bool pressed) {
		return pressed ? WindowMouseButtonState::Pressed : WindowMouseButtonState::Released;
	}

	WindowMouseScrollDirection SDL3Window::_ToWindowMouseScrollDirection(float x, float y) {
		if (y > 0) return WindowMouseScrollDirection::YPositive; // Scroll up
		if (y < 0) return WindowMouseScrollDirection::YNegative; // Scroll down
		if (x > 0) return WindowMouseScrollDirection::XPositive; // Scroll right
		if (x < 0) return WindowMouseScrollDirection::XNegative; // Scroll left
		return WindowMouseScrollDirection::None;
	}

	WindowKeyboardKeyState SDL3Window::_ToWindowKeyboardState(bool pressed, bool repeat) {
		if (!pressed) return WindowKeyboardKeyState::Released;
		
		return repeat ? WindowKeyboardKeyState::Held : WindowKeyboardKeyState::Pressed;
	}

	WindowKeyboardKey SDL3Window::_ToWindowKeyboardKey(SDL_Scancode scancode) {
		switch (scancode) {
			// A-Z keys
		case SDL_SCANCODE_A: return WindowKeyboardKey::A;
		case SDL_SCANCODE_B: return WindowKeyboardKey::B;
		case SDL_SCANCODE_C: return WindowKeyboardKey::C;
		case SDL_SCANCODE_D: return WindowKeyboardKey::D;
		case SDL_SCANCODE_E: return WindowKeyboardKey::E;
		case SDL_SCANCODE_F: return WindowKeyboardKey::F;
		case SDL_SCANCODE_G: return WindowKeyboardKey::G;
		case SDL_SCANCODE_H: return WindowKeyboardKey::H;
		case SDL_SCANCODE_I: return WindowKeyboardKey::I;
		case SDL_SCANCODE_J: return WindowKeyboardKey::J;
		case SDL_SCANCODE_K: return WindowKeyboardKey::K;
		case SDL_SCANCODE_L: return WindowKeyboardKey::L;
		case SDL_SCANCODE_M: return WindowKeyboardKey::M;
		case SDL_SCANCODE_N: return WindowKeyboardKey::N;
		case SDL_SCANCODE_O: return WindowKeyboardKey::O;
		case SDL_SCANCODE_P: return WindowKeyboardKey::P;
		case SDL_SCANCODE_Q: return WindowKeyboardKey::Q;
		case SDL_SCANCODE_R: return WindowKeyboardKey::R;
		case SDL_SCANCODE_S: return WindowKeyboardKey::S;
		case SDL_SCANCODE_T: return WindowKeyboardKey::T;
		case SDL_SCANCODE_U: return WindowKeyboardKey::U;
		case SDL_SCANCODE_V: return WindowKeyboardKey::V;
		case SDL_SCANCODE_W: return WindowKeyboardKey::W;
		case SDL_SCANCODE_X: return WindowKeyboardKey::X;
		case SDL_SCANCODE_Y: return WindowKeyboardKey::Y;
		case SDL_SCANCODE_Z: return WindowKeyboardKey::Z;
			// 0-9 keys
		case SDL_SCANCODE_0: return WindowKeyboardKey::Zero;
		case SDL_SCANCODE_1: return WindowKeyboardKey::One;
		case SDL_SCANCODE_2: return WindowKeyboardKey::Two;
		case SDL_SCANCODE_3: return WindowKeyboardKey::Three;
		case SDL_SCANCODE_4: return WindowKeyboardKey::Four;
		case SDL_SCANCODE_5: return WindowKeyboardKey::Five;
		case SDL_SCANCODE_6: return WindowKeyboardKey::Six;
		case SDL_SCANCODE_7: return WindowKeyboardKey::Seven;
		case SDL_SCANCODE_8: return WindowKeyboardKey::Eight;
		case SDL_SCANCODE_9: return WindowKeyboardKey::Nine;
			// Function keys
		case SDL_SCANCODE_F1: return WindowKeyboardKey::F1;
		case SDL_SCANCODE_F2: return WindowKeyboardKey::F2;
		case SDL_SCANCODE_F3: return WindowKeyboardKey::F3;
		case SDL_SCANCODE_F4: return WindowKeyboardKey::F4;
		case SDL_SCANCODE_F5: return WindowKeyboardKey::F5;
		case SDL_SCANCODE_F6: return WindowKeyboardKey::F6;
		case SDL_SCANCODE_F7: return WindowKeyboardKey::F7;
		case SDL_SCANCODE_F8: return WindowKeyboardKey::F8;
		case SDL_SCANCODE_F9: return WindowKeyboardKey::F9;
		case SDL_SCANCODE_F10: return WindowKeyboardKey::F10;
		case SDL_SCANCODE_F11: return WindowKeyboardKey::F11;
		case SDL_SCANCODE_F12: return WindowKeyboardKey::F12;
		case SDL_SCANCODE_F13: return WindowKeyboardKey::F13;
		case SDL_SCANCODE_F14: return WindowKeyboardKey::F14;
		case SDL_SCANCODE_F15: return WindowKeyboardKey::F15;
		case SDL_SCANCODE_F16: return WindowKeyboardKey::F16;
		case SDL_SCANCODE_F17: return WindowKeyboardKey::F17;
		case SDL_SCANCODE_F18: return WindowKeyboardKey::F18;
		case SDL_SCANCODE_F19: return WindowKeyboardKey::F19;
		case SDL_SCANCODE_F20: return WindowKeyboardKey::F20;
		case SDL_SCANCODE_F21: return WindowKeyboardKey::F21;
		case SDL_SCANCODE_F22: return WindowKeyboardKey::F22;
		case SDL_SCANCODE_F23: return WindowKeyboardKey::F23;
		case SDL_SCANCODE_F24: return WindowKeyboardKey::F24;
			// Non alpha-numeric keys
		case SDL_SCANCODE_GRAVE: return WindowKeyboardKey::GraveAccent;
		case SDL_SCANCODE_MINUS: return WindowKeyboardKey::Minus;
		case SDL_SCANCODE_EQUALS: return WindowKeyboardKey::Equal;
		case SDL_SCANCODE_LEFTBRACKET: return WindowKeyboardKey::LeftBracket;
		case SDL_SCANCODE_RIGHTBRACKET: return WindowKeyboardKey::RightBracket;
		case SDL_SCANCODE_BACKSLASH: return WindowKeyboardKey::Backslash;
		case SDL_SCANCODE_SEMICOLON: return WindowKeyboardKey::Semicolon;
		case SDL_SCANCODE_APOSTROPHE: return WindowKeyboardKey::Apostrophe;
		case SDL_SCANCODE_NONUSHASH: return WindowKeyboardKey::NonUS_Hash;
		case SDL_SCANCODE_NONUSBACKSLASH: return WindowKeyboardKey::NonUS_Backslash;
		case SDL_SCANCODE_COMMA: return WindowKeyboardKey::Comma;
		case SDL_SCANCODE_PERIOD: return WindowKeyboardKey::Period;
		case SDL_SCANCODE_SLASH: return WindowKeyboardKey::Slash;
		case SDL_SCANCODE_SPACE: return WindowKeyboardKey::Space;
			// Non-printable keys
		case SDL_SCANCODE_ESCAPE: return WindowKeyboardKey::Escape;
		case SDL_SCANCODE_TAB: return WindowKeyboardKey::Tab;
		case SDL_SCANCODE_RETURN: return WindowKeyboardKey::Enter;
			//Control keys
		case SDL_SCANCODE_BACKSPACE: return WindowKeyboardKey::Backspace;
		case SDL_SCANCODE_CAPSLOCK: return WindowKeyboardKey::CapsLock;
		case SDL_SCANCODE_LSHIFT: return WindowKeyboardKey::LeftShift;
		case SDL_SCANCODE_RSHIFT: return WindowKeyboardKey::RightShift;
		case SDL_SCANCODE_LCTRL: return WindowKeyboardKey::LeftControl;
		case SDL_SCANCODE_RCTRL: return WindowKeyboardKey::RightControl;
		case SDL_SCANCODE_LALT: return WindowKeyboardKey::LeftAlt;
		case SDL_SCANCODE_RALT: return WindowKeyboardKey::RightAlt;
		case SDL_SCANCODE_LGUI: return WindowKeyboardKey::LeftSuper;
		case SDL_SCANCODE_RGUI: return WindowKeyboardKey::RightSuper;
			// Navigation keys
		case SDL_SCANCODE_PRINTSCREEN: return WindowKeyboardKey::PrintScreen;
		case SDL_SCANCODE_SCROLLLOCK: return WindowKeyboardKey::ScrollLock;
		case SDL_SCANCODE_PAUSE: return WindowKeyboardKey::Pause;
		case SDL_SCANCODE_INSERT: return WindowKeyboardKey::Insert;
		case SDL_SCANCODE_DELETE: return WindowKeyboardKey::Delete;
		case SDL_SCANCODE_HOME: return WindowKeyboardKey::Home;
		case SDL_SCANCODE_END: return WindowKeyboardKey::End;
		case SDL_SCANCODE_PAGEUP: return WindowKeyboardKey::PageUp;
		case SDL_SCANCODE_PAGEDOWN: return WindowKeyboardKey::PageDown;
		case SDL_SCANCODE_UP: return WindowKeyboardKey::Up;
		case SDL_SCANCODE_DOWN: return WindowKeyboardKey::Down;
		case SDL_SCANCODE_LEFT: return WindowKeyboardKey::Left;
		case SDL_SCANCODE_RIGHT: return WindowKeyboardKey::Right;
			// Numpad keys
		case SDL_SCANCODE_KP_0: return WindowKeyboardKey::Numpad0;
		case SDL_SCANCODE_KP_1: return WindowKeyboardKey::Numpad1;
		case SDL_SCANCODE_KP_2: return WindowKeyboardKey::Numpad2;
		case SDL_SCANCODE_KP_3: return WindowKeyboardKey::Numpad3;
		case SDL_SCANCODE_KP_4: return WindowKeyboardKey::Numpad4;
		case SDL_SCANCODE_KP_5: return WindowKeyboardKey::Numpad5;
		case SDL_SCANCODE_KP_6: return WindowKeyboardKey::Numpad6;
		case SDL_SCANCODE_KP_7: return WindowKeyboardKey::Numpad7;
		case SDL_SCANCODE_KP_8: return WindowKeyboardKey::Numpad8;
		case SDL_SCANCODE_KP_9: return WindowKeyboardKey::Numpad9;
		case SDL_SCANCODE_KP_PERIOD: return WindowKeyboardKey::NumpadDecimal;
		case SDL_SCANCODE_KP_DIVIDE: return WindowKeyboardKey::NumpadDivide;
		case SDL_SCANCODE_KP_MULTIPLY: return WindowKeyboardKey::NumpadMultiply;
		case SDL_SCANCODE_KP_MINUS: return WindowKeyboardKey::NumpadSubtract;
		case SDL_SCANCODE_KP_PLUS: return WindowKeyboardKey::NumpadAdd;
		case SDL_SCANCODE_KP_ENTER: return WindowKeyboardKey::NumpadEnter;
		case SDL_SCANCODE_NUMLOCKCLEAR: return WindowKeyboardKey::NumLock;
		case SDL_SCANCODE_KP_EQUALS: return WindowKeyboardKey::NumpadEqual;

			// Add more mappings as needed for other keys
		case SDL_SCANCODE_APPLICATION: return WindowKeyboardKey::Menu;

		default:
			return WindowKeyboardKey::Unknow; // Unknown key
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