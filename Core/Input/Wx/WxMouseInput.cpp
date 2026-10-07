#include <Input/Wx/WxMouseInput.hpp>

namespace CoreEngine::Input::Wx {

    WxMouseInput::WxMouseInput() {
        SetPosition(0.0f, 0.0f);
        SetDelta(0.0f, 0.0f);
        SetScroll(0.0f, 0.0f);
        for (auto button : MouseInput::sMouseButtons) {
            mHeld[button] = false;
            mWasPressed[button] = false;
            mJustReleased[button] = false;
        }
    }

    bool WxMouseInput::CheckIsPressed(MouseButton button) {
        auto it = mHeld.find(button);
        return it != mHeld.end() && it->second;
    }

    bool WxMouseInput::CheckIsReleased(MouseButton button) {
        return !CheckIsPressed(button);
    }

    bool WxMouseInput::CheckIsJustPressed(MouseButton button) {
        auto itHeld = mHeld.find(button);
        bool isHeld = itHeld != mHeld.end() && itHeld->second;
        auto itWas = mWasPressed.find(button);
        bool wasPressed = itWas != mWasPressed.end() && itWas->second;
        return isHeld && !wasPressed;
    }

    MouseButtonState WxMouseInput::GetMouseButtonState(MouseButton button) {
        if (CheckIsPressed(button)) return MouseButtonState::Pressed;
        auto itRel = mJustReleased.find(button);
        if (itRel != mJustReleased.end() && itRel->second) return MouseButtonState::Released;
        return MouseButtonState::None;
    }

    void WxMouseInput::NotifyButtonDown(MouseButton button) {
        mHeld[button] = true;
        mJustReleased[button] = false;
    }

    void WxMouseInput::NotifyButtonUp(MouseButton button) {
        mHeld[button] = false;
        mJustReleased[button] = true;
    }

    void WxMouseInput::Update() {
        for (auto button : MouseInput::sMouseButtons) {
            mWasPressed[button] = CheckIsPressed(button);
            mJustReleased[button] = false;
        }
        // Reset per-frame positional deltas and scroll
        SetDelta(0.0f, 0.0f);
        SetScroll(0.0f, 0.0f);
    }

} // namespace CoreEngine::Input::Wx
