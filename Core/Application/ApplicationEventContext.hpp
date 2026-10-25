#pragma once
#include <pch.h>
#include <Event/EventContext.hpp>

namespace CoreEngine {
	class CORE_API WindowResizeEventContext : public Event::EventContext {
	public:
		WindowResizeEventContext(unsigned int width, unsigned int height) : mWidth(width), mHeight(height) {}
		CORE_FORCE_INLINE unsigned int GetWidth() const { return mWidth; }
		CORE_FORCE_INLINE unsigned int GetHeight() const { return mHeight; }

	private:
		unsigned int mWidth, mHeight;
	};

	enum class CORE_API KeyboardKey {
        Unknow,
        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Semicolon,
        Equal,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        World1,
        World2,


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

    enum class CORE_API KeyboardKeyState {
        None,
        Pressed,
        Held,
        Released
    };

    class CORE_API KeyBoardKeyEventContext : public Event::EventContext {
    public:
        KeyBoardKeyEventContext(KeyboardKey key, KeyboardKeyState state): mKey(key), mState(state) { }
        CORE_FORCE_INLINE KeyboardKey GetKey() const { return mKey; }
        CORE_FORCE_INLINE KeyboardKeyState GetState() const { return mState; }

    private:
        KeyboardKey mKey;
        KeyboardKeyState mState;
    };

    enum class CORE_API MouseButton {
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

    enum class CORE_API MouseButtonState {
        None,
        Pressed,
        Held,
        Released
    };

    class CORE_API MouseButtonEventContext : public Event::EventContext {
    public:
        MouseButtonEventContext(MouseButton button, MouseButtonState state) : mButton(button), mState(state) { }
        CORE_FORCE_INLINE MouseButton GetButton() const { return mButton; }
        CORE_FORCE_INLINE MouseButtonState GetButtonState() const { return mState; }
    private:
        MouseButton mButton;
        MouseButtonState mState;
    };

    class CORE_API MouseScrollEventContext : public Event::EventContext {
    public:
        MouseScrollEventContext(double deltaX, double deltaY): mDeltaX(deltaX), mDeltaY(deltaY) { }
        CORE_FORCE_INLINE double GetDeltaX() const { return mDeltaX; }
        CORE_FORCE_INLINE double GetDeltaY() const { return mDeltaY; }
    private:
        double mDeltaX, mDeltaY;
    };
}