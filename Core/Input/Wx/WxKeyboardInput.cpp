#include <Input/Wx/WxKeyboardInput.hpp>
#include <wx/defs.h>

namespace CoreEngine::Input::Wx {

    WxKeyboardInput::WxKeyboardInput() {
        for (KeyboardKey key : KeyboardInput::sKeyboardKeys) {
            mHeld[key] = false;
            mWasPressed[key] = false;
            mJustReleased[key] = false;
        }
    }

    bool WxKeyboardInput::CheckIsPressed(KeyboardKey key) {
        auto it = mHeld.find(key);
        return it != mHeld.end() && it->second;
    }

    bool WxKeyboardInput::CheckIsReleased(KeyboardKey key) {
        return !CheckIsPressed(key);
    }

    bool WxKeyboardInput::CheckIsJustPressed(KeyboardKey key) {
        auto itHeld = mHeld.find(key);
        bool isHeld = itHeld != mHeld.end() && itHeld->second;
        auto itWas = mWasPressed.find(key);
        bool wasPressed = itWas != mWasPressed.end() && itWas->second;
        return isHeld && !wasPressed;
    }

    KeyState WxKeyboardInput::GetKeyState(KeyboardKey key) {
        if (CheckIsPressed(key)) return KeyState::Pressed;
        auto itRel = mJustReleased.find(key);
        if (itRel != mJustReleased.end() && itRel->second) return KeyState::Released;
        return KeyState::None;
    }

    void WxKeyboardInput::NotifyKeyDown(int wxKeyCode) {
        KeyboardKey key = ToKeyboardKey(wxKeyCode);
        if (key == KeyboardKey::Unknow) return;

        mHeld[key] = true;
        mJustReleased[key] = false;
    }

    void WxKeyboardInput::NotifyKeyUp(int wxKeyCode) {
        KeyboardKey key = ToKeyboardKey(wxKeyCode);
        if (key == KeyboardKey::Unknow) return;

        mHeld[key] = false;
        mJustReleased[key] = true;
    }

    void WxKeyboardInput::Update() {
        for (KeyboardKey key : KeyboardInput::sKeyboardKeys) {
            mWasPressed[key] = CheckIsPressed(key);
            mJustReleased[key] = false;
        }
    }

    // -------------------------------------------------------------------------
    // wxKeyCode → KeyboardKey mapping
    // -------------------------------------------------------------------------

    KeyboardKey WxKeyboardInput::ToKeyboardKey(int kc) {
        // Letters (wxWidgets returns uppercase ASCII for letters)
        if (kc >= 'A' && kc <= 'Z') {
            // KeyboardKey::A == 0th letter; enum starts after symbols
            static const KeyboardKey letters[] = {
                KeyboardKey::A, KeyboardKey::B, KeyboardKey::C, KeyboardKey::D,
                KeyboardKey::E, KeyboardKey::F, KeyboardKey::G, KeyboardKey::H,
                KeyboardKey::I, KeyboardKey::J, KeyboardKey::K, KeyboardKey::L,
                KeyboardKey::M, KeyboardKey::N, KeyboardKey::O, KeyboardKey::P,
                KeyboardKey::Q, KeyboardKey::R, KeyboardKey::S, KeyboardKey::T,
                KeyboardKey::U, KeyboardKey::V, KeyboardKey::W, KeyboardKey::X,
                KeyboardKey::Y, KeyboardKey::Z
            };
            return letters[kc - 'A'];
        }
        // Digits
        if (kc >= '0' && kc <= '9') {
            static const KeyboardKey digits[] = {
                KeyboardKey::Zero,  KeyboardKey::One,   KeyboardKey::Two,
                KeyboardKey::Three, KeyboardKey::Four,  KeyboardKey::Five,
                KeyboardKey::Six,   KeyboardKey::Seven, KeyboardKey::Eight,
                KeyboardKey::Nine
            };
            return digits[kc - '0'];
        }
        // Function keys (wxWidgets defines WXK_F1 through WXK_F24)
        if (kc >= WXK_F1 && kc <= WXK_F24) {
            static const KeyboardKey fkeys[] = {
                KeyboardKey::F1,  KeyboardKey::F2,  KeyboardKey::F3,  KeyboardKey::F4,
                KeyboardKey::F5,  KeyboardKey::F6,  KeyboardKey::F7,  KeyboardKey::F8,
                KeyboardKey::F9,  KeyboardKey::F10, KeyboardKey::F11, KeyboardKey::F12,
                KeyboardKey::F13, KeyboardKey::F14, KeyboardKey::F15, KeyboardKey::F16,
                KeyboardKey::F17, KeyboardKey::F18, KeyboardKey::F19, KeyboardKey::F20,
                KeyboardKey::F21, KeyboardKey::F22, KeyboardKey::F23, KeyboardKey::F24
            };
            int idx = kc - WXK_F1;
            if (idx >= 0 && idx < 24) return fkeys[idx];
        }
        // Numpad
        if (kc >= WXK_NUMPAD0 && kc <= WXK_NUMPAD9) {
            static const KeyboardKey np[] = {
                KeyboardKey::Numpad0, KeyboardKey::Numpad1, KeyboardKey::Numpad2,
                KeyboardKey::Numpad3, KeyboardKey::Numpad4, KeyboardKey::Numpad5,
                KeyboardKey::Numpad6, KeyboardKey::Numpad7, KeyboardKey::Numpad8,
                KeyboardKey::Numpad9
            };
            return np[kc - WXK_NUMPAD0];
        }

        switch (kc) {
        case WXK_SPACE:         return KeyboardKey::Space;
        case WXK_ESCAPE:        return KeyboardKey::Escape;
        case WXK_RETURN:        return KeyboardKey::Enter;
        case WXK_TAB:           return KeyboardKey::Tab;
        case WXK_BACK:          return KeyboardKey::Backspace;
        case WXK_INSERT:        return KeyboardKey::Insert;
        case WXK_DELETE:        return KeyboardKey::Delete;
        case WXK_RIGHT:         return KeyboardKey::Right;
        case WXK_LEFT:          return KeyboardKey::Left;
        case WXK_DOWN:          return KeyboardKey::Down;
        case WXK_UP:            return KeyboardKey::Up;
        case WXK_PAGEUP:        return KeyboardKey::PageUp;
        case WXK_PAGEDOWN:      return KeyboardKey::PageDown;
        case WXK_HOME:          return KeyboardKey::Home;
        case WXK_END:           return KeyboardKey::End;
        case WXK_CAPITAL:       return KeyboardKey::CapsLock;
        case WXK_SCROLL:        return KeyboardKey::ScrollLock;
        case WXK_NUMLOCK:       return KeyboardKey::NumLock;
        case WXK_PRINT:         return KeyboardKey::PrintScreen;
        case WXK_PAUSE:         return KeyboardKey::Pause;
        case WXK_SHIFT:         return KeyboardKey::LeftShift;
        case WXK_CONTROL:       return KeyboardKey::LeftControl;
        case WXK_ALT:           return KeyboardKey::LeftAlt;
        case WXK_WINDOWS_LEFT:  return KeyboardKey::LeftSuper;
        case WXK_WINDOWS_RIGHT: return KeyboardKey::RightSuper;
        case WXK_MENU:          return KeyboardKey::Menu;
        case WXK_NUMPAD_DECIMAL:  return KeyboardKey::NumpadDecimal;
        case WXK_NUMPAD_DIVIDE:   return KeyboardKey::NumpadDivide;
        case WXK_NUMPAD_MULTIPLY: return KeyboardKey::NumpadMultiply;
        case WXK_NUMPAD_SUBTRACT: return KeyboardKey::NumpadSubtract;
        case WXK_NUMPAD_ADD:      return KeyboardKey::NumpadAdd;
        case WXK_NUMPAD_ENTER:    return KeyboardKey::NumpadEnter;
        case WXK_NUMPAD_EQUAL:    return KeyboardKey::NumpadEqual;
        // Punctuation — wxWidgets uses raw ASCII codes
        case '\'': return KeyboardKey::Apostrophe;
        case ',':  return KeyboardKey::Comma;
        case '-':  return KeyboardKey::Minus;
        case '.':  return KeyboardKey::Period;
        case '/':  return KeyboardKey::Slash;
        case ';':  return KeyboardKey::Semicolon;
        case '=':  return KeyboardKey::Equal;
        case '[':  return KeyboardKey::LeftBracket;
        case '\\': return KeyboardKey::Backslash;
        case ']':  return KeyboardKey::RightBracket;
        case '`':  return KeyboardKey::GraveAccent;
        default:   return KeyboardKey::Unknow;
        }
    }

} // namespace CoreEngine::Input::Wx
