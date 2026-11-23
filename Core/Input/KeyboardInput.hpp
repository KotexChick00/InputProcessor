#pragma once
#include <pch.h>
#include <Input/InputCodes.hpp>

namespace CoreEngine
{
    class CORE_API KeyboardInput
    {
    public:
        virtual ~KeyboardInput() = default;

        virtual bool CheckIsPressed(KeyboardKey key) = 0;
        virtual bool CheckIsReleased(KeyboardKey key) = 0;
        virtual bool CheckIsSinglePressed(KeyboardKey key) = 0;

        virtual KeyState GetKeyState(KeyboardKey key) = 0;

    protected:
        // Helpers help iterator over enum
        inline static std::vector<KeyboardKey> sKeyboardKeys = {
            KeyboardKey::Unknow,
            KeyboardKey::Space,
            KeyboardKey::Apostrophe,
            KeyboardKey::Comma,
            KeyboardKey::Minus,
            KeyboardKey::Period,
            KeyboardKey::Slash,
            KeyboardKey::Semicolon,
            KeyboardKey::Equal,
            KeyboardKey::LeftBracket,
            KeyboardKey::Backslash,
            KeyboardKey::RightBracket,
            KeyboardKey::GraveAccent,
            KeyboardKey::World1,
            KeyboardKey::World2,

            // Numbers
            KeyboardKey::Zero,
            KeyboardKey::One,
            KeyboardKey::Two,
            KeyboardKey::Three,
            KeyboardKey::Four,
            KeyboardKey::Five,
            KeyboardKey::Six,
            KeyboardKey::Seven,
            KeyboardKey::Eight,
            KeyboardKey::Nine,

            // Characters
            KeyboardKey::A,
            KeyboardKey::B,
            KeyboardKey::C,
            KeyboardKey::D,
            KeyboardKey::E,
            KeyboardKey::F,
            KeyboardKey::G,
            KeyboardKey::H,
            KeyboardKey::I,
            KeyboardKey::J,
            KeyboardKey::K,
            KeyboardKey::L,
            KeyboardKey::M,
            KeyboardKey::N,
            KeyboardKey::O,
            KeyboardKey::P,
            KeyboardKey::Q,
            KeyboardKey::R,
            KeyboardKey::S,
            KeyboardKey::T,
            KeyboardKey::U,
            KeyboardKey::V,
            KeyboardKey::W,
            KeyboardKey::X,
            KeyboardKey::Y,
            KeyboardKey::Z,

            // Controls & Navigations
            KeyboardKey::Escape,
            KeyboardKey::Enter,
            KeyboardKey::Tab,
            KeyboardKey::Backspace,
            KeyboardKey::Insert,
            KeyboardKey::Delete,
            KeyboardKey::Right,
            KeyboardKey::Left,
            KeyboardKey::Down,
            KeyboardKey::Up,
            KeyboardKey::PageUp,
            KeyboardKey::PageDown,
            KeyboardKey::Home,
            KeyboardKey::End,
            KeyboardKey::CapsLock,
            KeyboardKey::ScrollLock,
            KeyboardKey::NumLock,
            KeyboardKey::PrintScreen,
            KeyboardKey::Pause,

            // Function keys
            KeyboardKey::F1,
            KeyboardKey::F2,
            KeyboardKey::F3,
            KeyboardKey::F4,
            KeyboardKey::F5,
            KeyboardKey::F6,
            KeyboardKey::F7,
            KeyboardKey::F8,
            KeyboardKey::F9,
            KeyboardKey::F10,
            KeyboardKey::F11,
            KeyboardKey::F12,
            KeyboardKey::F13,
            KeyboardKey::F14,
            KeyboardKey::F15,
            KeyboardKey::F16,
            KeyboardKey::F17,
            KeyboardKey::F18,
            KeyboardKey::F19,
            KeyboardKey::F20,
            KeyboardKey::F21,
            KeyboardKey::F22,
            KeyboardKey::F23,
            KeyboardKey::F24,
            KeyboardKey::F25,

            // Numpad
            KeyboardKey::Numpad0,
            KeyboardKey::Numpad1,
            KeyboardKey::Numpad2,
            KeyboardKey::Numpad3,
            KeyboardKey::Numpad4,
            KeyboardKey::Numpad5,
            KeyboardKey::Numpad6,
            KeyboardKey::Numpad7,
            KeyboardKey::Numpad8,
            KeyboardKey::Numpad9,
            KeyboardKey::NumpadDecimal,
            KeyboardKey::NumpadDivide,
            KeyboardKey::NumpadMultiply,
            KeyboardKey::NumpadSubtract,
            KeyboardKey::NumpadAdd,
            KeyboardKey::NumpadEnter,
            KeyboardKey::NumpadEqual,

            // Modifiers
            KeyboardKey::LeftShift,
            KeyboardKey::LeftControl,
            KeyboardKey::LeftAlt,
            KeyboardKey::LeftSuper, // Window left key
            KeyboardKey::RightShift,
            KeyboardKey::RightControl,
            KeyboardKey::RightAlt,
            KeyboardKey::RightSuper, // Window right key
            KeyboardKey::Menu};
    };

    class IKeyboardInputCheckIsHeldable
    {
    public:
        virtual bool CheckIsHeld(KeyboardKey key) = 0;
    };
}