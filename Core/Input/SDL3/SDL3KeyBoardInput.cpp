#include <Input/SDL3/SDL3KeyBoardInput.h>

namespace CoreEngine::Input::SDL3 {
	namespace {
		inline bool Valid(SDL_Scancode scancode) {
			return scancode >= SDL_SCANCODE_UNKNOWN && scancode < SDL_SCANCODE_COUNT;
		}
	}

	SDL3KeyBoardInput::SDL3KeyBoardInput(SDL_Window* window) : mWindow(window) {}

	void SDL3KeyBoardInput::Update() {
		int count = 0;
		const bool* state = SDL_GetKeyboardState(&count);
		mPrev = mCurr;
		for (int i = 0; i < count && i < static_cast<int>(mCurr.size()); ++i)
			mCurr[i] = state[i];
	}

	bool SDL3KeyBoardInput::CheckIsPressed(KeyboardKey key) {
		SDL_Scancode scancode = ToSDL3Scancode(key);
		if (!Valid(scancode)) return false;
		return mCurr[scancode] && !mPrev[scancode];
	}

	bool SDL3KeyBoardInput::CheckIsReleased(KeyboardKey key) {
		SDL_Scancode scancode = ToSDL3Scancode(key);
		if (!Valid(scancode)) return false;
		return !mCurr[scancode] && mPrev[scancode];
	}

	bool SDL3KeyBoardInput::CheckIsHeld(KeyboardKey key) {
		SDL_Scancode scancode = ToSDL3Scancode(key);
		if (!Valid(scancode)) return false;
		return mCurr[scancode];
	}

	KeyState SDL3KeyBoardInput::GetKeyState(KeyboardKey key) {
		if (CheckIsPressed(key)) return KeyState::Pressed;
		if (CheckIsReleased(key)) return KeyState::Released;
		if (CheckIsHeld(key)) return KeyState::Held;
		return KeyState::None;
	}

	SDL_Scancode SDL3KeyBoardInput::ToSDL3Scancode(KeyboardKey key) {
		switch (key) {
		case KeyboardKey::Unknow: return SDL_SCANCODE_UNKNOWN;
		case KeyboardKey::Space: return SDL_SCANCODE_SPACE;
		case KeyboardKey::Apostrophe: return SDL_SCANCODE_APOSTROPHE;
		case KeyboardKey::Comma: return SDL_SCANCODE_COMMA;
		case KeyboardKey::Minus: return SDL_SCANCODE_MINUS;
		case KeyboardKey::Period: return SDL_SCANCODE_PERIOD;
		case KeyboardKey::Slash: return SDL_SCANCODE_SLASH;
		case KeyboardKey::Semicolon: return SDL_SCANCODE_SEMICOLON;
		case KeyboardKey::Equal: return SDL_SCANCODE_EQUALS;
		case KeyboardKey::LeftBracket: return SDL_SCANCODE_LEFTBRACKET;
		case KeyboardKey::Backslash: return SDL_SCANCODE_BACKSLASH;
		case KeyboardKey::RightBracket: return SDL_SCANCODE_RIGHTBRACKET;
		case KeyboardKey::GraveAccent: return SDL_SCANCODE_GRAVE;
		//case KeyboardKey::World1: return SDL_SCANCODE_UNKNOWN;
		//case KeyboardKey::World2: return SDL_SCANCODE_UNKNOWN;
		case KeyboardKey::NonUS_Backslash: return SDL_SCANCODE_NONUSBACKSLASH;
		case KeyboardKey::NonUS_Hash: return SDL_SCANCODE_NONUSHASH;

		case KeyboardKey::Zero: return SDL_SCANCODE_0;
		case KeyboardKey::One: return SDL_SCANCODE_1;
		case KeyboardKey::Two: return SDL_SCANCODE_2;
		case KeyboardKey::Three: return SDL_SCANCODE_3;
		case KeyboardKey::Four: return SDL_SCANCODE_4;
		case KeyboardKey::Five: return SDL_SCANCODE_5;
		case KeyboardKey::Six: return SDL_SCANCODE_6;
		case KeyboardKey::Seven: return SDL_SCANCODE_7;
		case KeyboardKey::Eight: return SDL_SCANCODE_8;
		case KeyboardKey::Nine: return SDL_SCANCODE_9;

		case KeyboardKey::A: return SDL_SCANCODE_A;
		case KeyboardKey::B: return SDL_SCANCODE_B;
		case KeyboardKey::C: return SDL_SCANCODE_C;
		case KeyboardKey::D: return SDL_SCANCODE_D;
		case KeyboardKey::E: return SDL_SCANCODE_E;
		case KeyboardKey::F: return SDL_SCANCODE_F;
		case KeyboardKey::G: return SDL_SCANCODE_G;
		case KeyboardKey::H: return SDL_SCANCODE_H;
		case KeyboardKey::I: return SDL_SCANCODE_I;
		case KeyboardKey::J: return SDL_SCANCODE_J;
		case KeyboardKey::K: return SDL_SCANCODE_K;
		case KeyboardKey::L: return SDL_SCANCODE_L;
		case KeyboardKey::M: return SDL_SCANCODE_M;
		case KeyboardKey::N: return SDL_SCANCODE_N;
		case KeyboardKey::O: return SDL_SCANCODE_O;
		case KeyboardKey::P: return SDL_SCANCODE_P;
		case KeyboardKey::Q: return SDL_SCANCODE_Q;
		case KeyboardKey::R: return SDL_SCANCODE_R;
		case KeyboardKey::S: return SDL_SCANCODE_S;
		case KeyboardKey::T: return SDL_SCANCODE_T;
		case KeyboardKey::U: return SDL_SCANCODE_U;
		case KeyboardKey::V: return SDL_SCANCODE_V;
		case KeyboardKey::W: return SDL_SCANCODE_W;
		case KeyboardKey::X: return SDL_SCANCODE_X;
		case KeyboardKey::Y: return SDL_SCANCODE_Y;
		case KeyboardKey::Z: return SDL_SCANCODE_Z;

		case KeyboardKey::Escape: return SDL_SCANCODE_ESCAPE;
		case KeyboardKey::Enter: return SDL_SCANCODE_RETURN;
		case KeyboardKey::Tab: return SDL_SCANCODE_TAB;
		case KeyboardKey::Backspace: return SDL_SCANCODE_BACKSPACE;
		case KeyboardKey::Insert: return SDL_SCANCODE_INSERT;
		case KeyboardKey::Delete: return SDL_SCANCODE_DELETE;
		case KeyboardKey::Right: return SDL_SCANCODE_RIGHT;
		case KeyboardKey::Left: return SDL_SCANCODE_LEFT;
		case KeyboardKey::Down: return SDL_SCANCODE_DOWN;
		case KeyboardKey::Up: return SDL_SCANCODE_UP;
		case KeyboardKey::PageUp: return SDL_SCANCODE_PAGEUP;
		case KeyboardKey::PageDown: return SDL_SCANCODE_PAGEDOWN;
		case KeyboardKey::Home: return SDL_SCANCODE_HOME;
		case KeyboardKey::End: return SDL_SCANCODE_END;
		case KeyboardKey::CapsLock: return SDL_SCANCODE_CAPSLOCK;
		case KeyboardKey::ScrollLock: return SDL_SCANCODE_SCROLLLOCK;
		case KeyboardKey::NumLock: return SDL_SCANCODE_NUMLOCKCLEAR;
		case KeyboardKey::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
		case KeyboardKey::Pause: return SDL_SCANCODE_PAUSE;


		case KeyboardKey::F1: return SDL_SCANCODE_F1;
		case KeyboardKey::F2: return SDL_SCANCODE_F2;
		case KeyboardKey::F3: return SDL_SCANCODE_F3;
		case KeyboardKey::F4: return SDL_SCANCODE_F4;
		case KeyboardKey::F5: return SDL_SCANCODE_F5;
		case KeyboardKey::F6: return SDL_SCANCODE_F6;
		case KeyboardKey::F7: return SDL_SCANCODE_F7;
		case KeyboardKey::F8: return SDL_SCANCODE_F8;
		case KeyboardKey::F9: return SDL_SCANCODE_F9;
		case KeyboardKey::F10: return SDL_SCANCODE_F10;
		case KeyboardKey::F11: return SDL_SCANCODE_F11;
		case KeyboardKey::F12: return SDL_SCANCODE_F12;
		case KeyboardKey::F13: return SDL_SCANCODE_F13;
		case KeyboardKey::F14: return SDL_SCANCODE_F14;
		case KeyboardKey::F15: return SDL_SCANCODE_F15;
		case KeyboardKey::F16: return SDL_SCANCODE_F16;
		case KeyboardKey::F17: return SDL_SCANCODE_F17;
		case KeyboardKey::F18: return SDL_SCANCODE_F18;
		case KeyboardKey::F19: return SDL_SCANCODE_F19;
		case KeyboardKey::F20: return SDL_SCANCODE_F20;
		case KeyboardKey::F21: return SDL_SCANCODE_F21;
		case KeyboardKey::F22: return SDL_SCANCODE_F22;
		case KeyboardKey::F23: return SDL_SCANCODE_F23;
		case KeyboardKey::F24: return SDL_SCANCODE_F24;
		// case KeyboardKey::F25: return SDL_SCANCODE_UNKNOWN;

		case KeyboardKey::Numpad0: return SDL_SCANCODE_KP_0;
		case KeyboardKey::Numpad1: return SDL_SCANCODE_KP_1;
		case KeyboardKey::Numpad2: return SDL_SCANCODE_KP_2;
		case KeyboardKey::Numpad3: return SDL_SCANCODE_KP_3;
		case KeyboardKey::Numpad4: return SDL_SCANCODE_KP_4;
		case KeyboardKey::Numpad5: return SDL_SCANCODE_KP_5;
		case KeyboardKey::Numpad6: return SDL_SCANCODE_KP_6;
		case KeyboardKey::Numpad7: return SDL_SCANCODE_KP_7;
		case KeyboardKey::Numpad8: return SDL_SCANCODE_KP_8;
		case KeyboardKey::Numpad9: return SDL_SCANCODE_KP_9;
		case KeyboardKey::NumpadDecimal: return SDL_SCANCODE_KP_PERIOD;
		case KeyboardKey::NumpadDivide: return SDL_SCANCODE_KP_DIVIDE;
		case KeyboardKey::NumpadMultiply: return SDL_SCANCODE_KP_MULTIPLY;
		case KeyboardKey::NumpadSubtract: return SDL_SCANCODE_KP_MINUS;
		case KeyboardKey::NumpadAdd: return SDL_SCANCODE_KP_PLUS;
		case KeyboardKey::NumpadEnter: return SDL_SCANCODE_KP_ENTER;
		case KeyboardKey::NumpadEqual: return SDL_SCANCODE_KP_EQUALS;


		case KeyboardKey::LeftShift: return SDL_SCANCODE_LSHIFT;
		case KeyboardKey::LeftControl: return SDL_SCANCODE_LCTRL;
		case KeyboardKey::LeftAlt: return SDL_SCANCODE_LALT;
		case KeyboardKey::LeftSuper: return SDL_SCANCODE_LGUI;
		case KeyboardKey::RightShift: return SDL_SCANCODE_RSHIFT;
		case KeyboardKey::RightControl: return SDL_SCANCODE_RCTRL;
		case KeyboardKey::RightAlt: return SDL_SCANCODE_RALT;
		case KeyboardKey::RightSuper: return SDL_SCANCODE_RGUI;
		case KeyboardKey::Menu: return SDL_SCANCODE_MENU;

		default: return SDL_SCANCODE_UNKNOWN;

		}
	}
}