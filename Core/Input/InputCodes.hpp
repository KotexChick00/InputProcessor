#pragma once
#include <pch.h>

// Platform-independent input codes shared by the polling input API
// (KeyboardInput / MouseInput) and the application events (Event/EventContext.hpp).
namespace CoreEngine
{
    enum class CORE_API KeyboardKey
    {
        // Special Keys and notations
        Unknow,
        Space,
        Apostrophe, /* The apostrophe (') key */
        Comma,
        Minus,
        Period, /* The period or full stop or dot (.) key */
        Slash,
        Semicolon,
        Equal,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        World1, /* Bounded to OEM-102 key */
        World2, /* Bounded to OEM-102 key */
                // Both World1 and World2 keys are bounded to OEM-102 key,
        // which is located between the left Shift and Z keys on a standard US keyboard layout.
        // Theirs equivalent key in SDL3 is named SDL_SCANCODE_NONUSBACKSLASH.
        //NonUS_Backslash, // The Non-US Backslash key is located between the left Shift and Z keys on a standard US keyboard layout.

        // Numbers in the upper character parts
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,

        // Characters
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,

        Numpad0,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        NumpadDecimal,
        NumpadDivide,
        NumpadMultiply,
        NumpadSubtract,
        NumpadAdd,
        NumpadEnter,
        NumpadEqual,

        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper, // Window left key
        RightShift,
        RightControl,
        RightAlt,
        RightSuper, // Window right key
        Menu
    };

    // Polling state returned by KeyboardInput::GetKeyState
    enum class CORE_API KeyState
    {
        None,
        Pressed,
        Released
    };

    // State carried by KeyBoardKeyEventContext
    enum class CORE_API KeyboardKeyState
    {
        None,
        Pressed,
        Held,
        Released
    };

    enum class CORE_API MouseButton
    {
        Button1, // Left
        Button2, // Right
        Button3, // Middle
        Button4, // Thumb back or Thumb button 1
        Button5, // Thumb forward or Thumb button 2
        Button6,
        Button7,
        Button8,
        ButtonLast = Button8,
        ButtonLeft = Button1,
        ButtonRight = Button2,
        ButtonMiddle = Button3
    };

    // Held is only reported by events; MouseInput::GetMouseButtonState returns None/Pressed/Released
    enum class CORE_API MouseButtonState
    {
        None,
        Pressed,
        Held,
        Released
    };
}
