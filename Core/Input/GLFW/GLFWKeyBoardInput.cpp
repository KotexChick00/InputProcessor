#include <Input/GLFW/GLFWKeyBoardInput.hpp>

namespace InputProcessor::Input::GLFW {
	GLFWKeyBoardInput::GLFWKeyBoardInput(GLFWwindow* window) : mWindow(window) { }

	bool GLFWKeyBoardInput::CheckIsPressed(KeyboardKey key) {
		int glfwKey = ToGLFWKey(key);
		if (glfwKey == GLFW_KEY_UNKNOWN || mWindow == nullptr) return false;
		int state = glfwGetKey(mWindow, glfwKey);
		return state == GLFW_PRESS;
	}

	bool GLFWKeyBoardInput::CheckIsReleased(KeyboardKey key) {
		int glfwKey = ToGLFWKey(key);
		if (glfwKey == GLFW_KEY_UNKNOWN || mWindow == nullptr) return false;
		int state = glfwGetKey(mWindow, glfwKey);
		return state == GLFW_RELEASE;
	}

	KeyState GLFWKeyBoardInput::GetKeyState(KeyboardKey key) {
		if (CheckIsPressed(key)) return KeyState::Pressed;
		if (CheckIsReleased(key)) return KeyState::Released;
		return KeyState::None;
	}

	int GLFWKeyBoardInput::ToGLFWKey(KeyboardKey key) {
		switch (key) {
		case KeyboardKey::Unknow: return GLFW_KEY_UNKNOWN;
		case KeyboardKey::Space: return GLFW_KEY_SPACE;
		case KeyboardKey::Apostrophe: return GLFW_KEY_APOSTROPHE;
		case KeyboardKey::Comma: return GLFW_KEY_COMMA;
		case KeyboardKey::Minus: return GLFW_KEY_MINUS;
		case KeyboardKey::Period: return GLFW_KEY_PERIOD;
		case KeyboardKey::Slash: return GLFW_KEY_SLASH;
		case KeyboardKey::Semicolon: return GLFW_KEY_SEMICOLON;
		case KeyboardKey::Equal: return GLFW_KEY_EQUAL;
		case KeyboardKey::LeftBracket: return GLFW_KEY_LEFT_BRACKET;
		case KeyboardKey::Backslash: return GLFW_KEY_BACKSLASH;
		case KeyboardKey::RightBracket: return GLFW_KEY_RIGHT_BRACKET;
		case KeyboardKey::GraveAccent: return GLFW_KEY_GRAVE_ACCENT;
		case KeyboardKey::World1: return GLFW_KEY_WORLD_1;
		case KeyboardKey::World2: return GLFW_KEY_WORLD_2;

		case KeyboardKey::Zero: return GLFW_KEY_0;
		case KeyboardKey::One: return GLFW_KEY_1;
		case KeyboardKey::Two: return GLFW_KEY_2;
		case KeyboardKey::Three: return GLFW_KEY_3;
		case KeyboardKey::Four: return GLFW_KEY_4;
		case KeyboardKey::Five: return GLFW_KEY_5;
		case KeyboardKey::Six: return GLFW_KEY_6;
		case KeyboardKey::Seven: return GLFW_KEY_7;
		case KeyboardKey::Eight: return GLFW_KEY_8;
		case KeyboardKey::Nine: return GLFW_KEY_9;

		case KeyboardKey::A: return GLFW_KEY_A;
		case KeyboardKey::B: return GLFW_KEY_B;
		case KeyboardKey::C: return GLFW_KEY_C;
		case KeyboardKey::D: return GLFW_KEY_D;
		case KeyboardKey::E: return GLFW_KEY_E;
		case KeyboardKey::F: return GLFW_KEY_F;
		case KeyboardKey::G: return GLFW_KEY_G;
		case KeyboardKey::H: return GLFW_KEY_H;
		case KeyboardKey::I: return GLFW_KEY_I;
		case KeyboardKey::J: return GLFW_KEY_J;
		case KeyboardKey::K: return GLFW_KEY_K;
		case KeyboardKey::L: return GLFW_KEY_L;
		case KeyboardKey::M: return GLFW_KEY_M;
		case KeyboardKey::N: return GLFW_KEY_N;
		case KeyboardKey::O: return GLFW_KEY_O;
		case KeyboardKey::P: return GLFW_KEY_P;
		case KeyboardKey::Q: return GLFW_KEY_Q;
		case KeyboardKey::R: return GLFW_KEY_R;
		case KeyboardKey::S: return GLFW_KEY_S;
		case KeyboardKey::T: return GLFW_KEY_T;
		case KeyboardKey::U: return GLFW_KEY_U;
		case KeyboardKey::V: return GLFW_KEY_V;
		case KeyboardKey::W: return GLFW_KEY_W;
		case KeyboardKey::X: return GLFW_KEY_X;
		case KeyboardKey::Y: return GLFW_KEY_Y;
		case KeyboardKey::Z: return GLFW_KEY_Z;

		case KeyboardKey::Escape: return GLFW_KEY_ESCAPE;
		case KeyboardKey::Enter: return GLFW_KEY_ENTER;
		case KeyboardKey::Tab: return GLFW_KEY_TAB;
		case KeyboardKey::Backspace: return GLFW_KEY_BACKSPACE;
		case KeyboardKey::Insert: return GLFW_KEY_INSERT;
		case KeyboardKey::Delete: return GLFW_KEY_DELETE;
		case KeyboardKey::Right: return GLFW_KEY_RIGHT;
		case KeyboardKey::Left: return GLFW_KEY_LEFT;
		case KeyboardKey::Down: return GLFW_KEY_DOWN;
		case KeyboardKey::Up: return GLFW_KEY_UP;
		case KeyboardKey::PageUp: return GLFW_KEY_PAGE_UP;
		case KeyboardKey::PageDown: return GLFW_KEY_PAGE_DOWN;
		case KeyboardKey::Home: return GLFW_KEY_HOME;
		case KeyboardKey::End: return GLFW_KEY_END;
		case KeyboardKey::CapsLock: return GLFW_KEY_CAPS_LOCK;
		case KeyboardKey::ScrollLock: return GLFW_KEY_SCROLL_LOCK;
		case KeyboardKey::NumLock: return GLFW_KEY_NUM_LOCK;
		case KeyboardKey::PrintScreen: return GLFW_KEY_PRINT_SCREEN;
		case KeyboardKey::Pause: return GLFW_KEY_PAUSE;

		case KeyboardKey::F1: return GLFW_KEY_F1;
		case KeyboardKey::F2: return GLFW_KEY_F2;
		case KeyboardKey::F3: return GLFW_KEY_F3;
		case KeyboardKey::F4: return GLFW_KEY_F4;
		case KeyboardKey::F5: return GLFW_KEY_F5;
		case KeyboardKey::F6: return GLFW_KEY_F6;
		case KeyboardKey::F7: return GLFW_KEY_F7;
		case KeyboardKey::F8: return GLFW_KEY_F8;
		case KeyboardKey::F9: return GLFW_KEY_F9;
		case KeyboardKey::F10: return GLFW_KEY_F10;
		case KeyboardKey::F11: return GLFW_KEY_F11;
		case KeyboardKey::F12: return GLFW_KEY_F12;
		case KeyboardKey::F13: return GLFW_KEY_F13;
		case KeyboardKey::F14: return GLFW_KEY_F14;
		case KeyboardKey::F15: return GLFW_KEY_F15;
		case KeyboardKey::F16: return GLFW_KEY_F16;
		case KeyboardKey::F17: return GLFW_KEY_F17;
		case KeyboardKey::F18: return GLFW_KEY_F18;
		case KeyboardKey::F19: return GLFW_KEY_F19;
		case KeyboardKey::F20: return GLFW_KEY_F20;
		case KeyboardKey::F21: return GLFW_KEY_F21;
		case KeyboardKey::F22: return GLFW_KEY_F22;
		case KeyboardKey::F23: return GLFW_KEY_F23;
		case KeyboardKey::F24: return GLFW_KEY_F24;
		case KeyboardKey::F25: return GLFW_KEY_F25;

		case KeyboardKey::Numpad0: return GLFW_KEY_KP_0;
		case KeyboardKey::Numpad1: return GLFW_KEY_KP_1;
		case KeyboardKey::Numpad2: return GLFW_KEY_KP_2;
		case KeyboardKey::Numpad3: return GLFW_KEY_KP_3;
		case KeyboardKey::Numpad4: return GLFW_KEY_KP_4;
		case KeyboardKey::Numpad5: return GLFW_KEY_KP_5;
		case KeyboardKey::Numpad6: return GLFW_KEY_KP_6;
		case KeyboardKey::Numpad7: return GLFW_KEY_KP_7;
		case KeyboardKey::Numpad8: return GLFW_KEY_KP_8;
		case KeyboardKey::Numpad9: return GLFW_KEY_KP_9;
		case KeyboardKey::NumpadDecimal: return GLFW_KEY_KP_DECIMAL;
		case KeyboardKey::NumpadDivide: return GLFW_KEY_KP_DIVIDE;
		case KeyboardKey::NumpadMultiply: return GLFW_KEY_KP_MULTIPLY;
		case KeyboardKey::NumpadSubtract: return GLFW_KEY_KP_SUBTRACT;
		case KeyboardKey::NumpadAdd: return GLFW_KEY_KP_ADD;
		case KeyboardKey::NumpadEnter: return GLFW_KEY_KP_ENTER;
		case KeyboardKey::NumpadEqual: return GLFW_KEY_KP_EQUAL;

		case KeyboardKey::LeftShift: return GLFW_KEY_LEFT_SHIFT;
		case KeyboardKey::LeftControl: return GLFW_KEY_LEFT_CONTROL;
		case KeyboardKey::LeftAlt: return GLFW_KEY_LEFT_ALT;
		case KeyboardKey::LeftSuper: return GLFW_KEY_LEFT_SUPER;
		case KeyboardKey::RightShift: return GLFW_KEY_RIGHT_SHIFT;
		case KeyboardKey::RightControl: return GLFW_KEY_RIGHT_CONTROL;
		case KeyboardKey::RightAlt: return GLFW_KEY_RIGHT_ALT;
		case KeyboardKey::RightSuper: return GLFW_KEY_RIGHT_SUPER;
		case KeyboardKey::Menu: return GLFW_KEY_MENU;

		default: return GLFW_KEY_UNKNOWN;
		}
	}
}