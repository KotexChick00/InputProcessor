#pragma once
#include <pch.h>
#include <Input/InputCodes.hpp>

namespace CoreEngine
{
    // Note: Marker interface
    class CORE_API EventContext
    {
    public:
        EventContext() = default;
        virtual ~EventContext() = default;
    };

    class CORE_API WindowResizeEventContext : public EventContext
    {
    public:
        WindowResizeEventContext(unsigned int width, unsigned int height)
            : mWidth(width)
            , mHeight(height)
        {
        }
        CORE_FORCE_INLINE unsigned int GetWidth() const { return mWidth; }
        CORE_FORCE_INLINE unsigned int GetHeight() const { return mHeight; }

    private:
        unsigned int mWidth, mHeight;
    };

    class CORE_API KeyBoardKeyEventContext : public EventContext
    {
    public:
        KeyBoardKeyEventContext(KeyboardKey key, KeyboardKeyState state)
            : mKey(key)
            , mState(state)
        {
        }
        CORE_FORCE_INLINE KeyboardKey GetKey() const { return mKey; }
        CORE_FORCE_INLINE KeyboardKeyState GetState() const { return mState; }

    private:
        KeyboardKey mKey;
        KeyboardKeyState mState;
    };

    class CORE_API MouseButtonEventContext : public EventContext
    {
    public:
        MouseButtonEventContext(MouseButton button, MouseButtonState state)
            : mButton(button)
            , mState(state)
        {
        }
        CORE_FORCE_INLINE MouseButton GetButton() const { return mButton; }
        CORE_FORCE_INLINE MouseButtonState GetButtonState() const { return mState; }

    private:
        MouseButton mButton;
        MouseButtonState mState;
    };

    class CORE_API MouseScrollEventContext : public EventContext
    {
    public:
        MouseScrollEventContext(double deltaX, double deltaY)
            : mDeltaX(deltaX)
            , mDeltaY(deltaY)
        {
        }
        CORE_FORCE_INLINE double GetDeltaX() const { return mDeltaX; }
        CORE_FORCE_INLINE double GetDeltaY() const { return mDeltaY; }

    private:
        double mDeltaX, mDeltaY;
    };
}
