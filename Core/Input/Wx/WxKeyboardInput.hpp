#pragma once
#include <Input/KeyboardInput.hpp>

namespace CoreEngine
{
    /// wxWidgets-backed KeyboardInput.
    ///
    /// Key state is maintained manually: WxWindow feeds raw wxKeyEvent data
    /// into NotifyKeyDown / NotifyKeyUp; Update() is called in EndFrame() to
    /// advance state machine and record wasPressed history.
    class WxKeyboardInput : public KeyboardInput
    {
    public:
        WxKeyboardInput();
        ~WxKeyboardInput() = default;

        bool CheckIsPressed(KeyboardKey key) override;
        bool CheckIsReleased(KeyboardKey key) override;
        bool CheckIsSinglePressed(KeyboardKey key) override;

        KeyState GetKeyState(KeyboardKey key) override;

        // Called by WxWindow event handlers
        void NotifyKeyDown(int wxKeyCode);
        void NotifyKeyUp(int wxKeyCode);

        // Called in EndFrame() — advances state machine
        void Update();

    private:
        std::unordered_map<KeyboardKey, bool> mHeld;
        std::unordered_map<KeyboardKey, bool> mWasPressed;
        std::unordered_map<KeyboardKey, bool> mJustReleased;
        // Set on a released -> pressed transition, cleared by Update(). Keeps a tap whose key-down and
        // key-up both arrive within one frame (otherwise "held now && not held last frame" misses it).
        std::unordered_map<KeyboardKey, bool> mPressedThisFrame;

        static KeyboardKey ToKeyboardKey(int wxKeyCode);
    };
} // namespace CoreEngine
