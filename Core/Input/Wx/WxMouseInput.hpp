#pragma once
#include <Input/MouseInput.hpp>

namespace CoreEngine::Input::Wx {
    /// wxWidgets-backed MouseInput.
    ///
    /// Mouse state is maintained by WxWindow calling Notify* methods from
    /// wx event handlers. Update() is called in EndFrame() to advance the
    /// state machine and reset per-frame deltas.
    class WxMouseInput : public MouseInput {
    public:
        WxMouseInput();
        ~WxMouseInput() = default;

        bool CheckIsPressed(MouseButton mouseButton) override;
        bool CheckIsReleased(MouseButton mouseButton) override;
        bool CheckIsJustPressed(MouseButton mouseButton) override;

        MouseButtonState GetMouseButtonState(MouseButton mouseButton) override;

        // Called by WxWindow event handlers
        void NotifyButtonDown(MouseButton button);
        void NotifyButtonUp(MouseButton button);

        // Called in EndFrame()
        void Update();

    private:
        std::unordered_map<MouseButton, bool> mHeld;
        std::unordered_map<MouseButton, bool> mWasPressed;
        std::unordered_map<MouseButton, bool> mJustReleased;
    };
} // namespace CoreEngine::Input::Wx
