#pragma once
#include <Input/InputState.hpp>
#include <pch.h>

namespace CoreEngine::Window {
    struct WindowConfiguration {
        unsigned int Width;
        unsigned int Height;
        std::string Title;
    };

    struct WindowMouseMoveEventContext {
        double XPos;
        double YPos;
    };

    enum class WindowMouseButton {
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

    enum class WindowMouseButtonState {
        None,
        Pressed,
        Held,
        Released
    };

    struct WindowMouseButtonEventContext {
        WindowMouseButton Button;
        WindowMouseButtonState State;
    };

    enum class WindowKeyboardKey {
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

    enum class WindowKeyboardKeyState {
        None,
        Pressed,
        Held,
        Released
    };

    struct WindowKeyboardKeyEventContext {
        WindowKeyboardKey Key;
        WindowKeyboardKeyState State;
    };

    struct WindowMouseScrollEventContext {
        // If touchpads, Trackball...
        double OffsetX;
        double OffsetY;
    };

    struct WindowResizeEventContext {
        unsigned int Width;
        unsigned int Height;
    };

	class CORE_API IWindowVisitor;

	class CORE_API IWindow {
	public:
		virtual void Init(const WindowConfiguration& config) = 0;
		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;
		virtual bool CheckShouldClose() = 0;
		virtual void Close() = 0;

		virtual void OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) = 0;
		virtual void OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) = 0;
		virtual void OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) = 0;
        virtual void OnMouseSrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) = 0;
        virtual void OnWindowReiszeEventCallback(std::function<void(const WindowResizeEventContext&)> callback) = 0;

        virtual Input::InputState GetInput() const = 0;

        virtual float GetCurrentSeconds() = 0;
		virtual void Accept(IWindowVisitor* visitor) = 0;

		virtual void EndFrame() = 0;
	};
}