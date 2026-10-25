#pragma once
#include <pch.h>
#include <chrono>
#include <Window/IWindow.hpp>
#include <Input/Wx/WxKeyboardInput.hpp>
#include <Input/Wx/WxMouseInput.hpp>

// Forward-declare wx types to keep compile cost down.
// Full headers are included only in WxWindow.cpp.
class wxFrame;
class wxGLCanvas;
class wxGLContext;
class wxEventLoopBase;
class wxEventLoopActivator;

namespace CoreEngine::Window::Wx {

    // Engine-owned render loop dispatches wx events through one active wx event loop.
    class CORE_API WxWindow : public IWindow {
    public:
        WxWindow();
        ~WxWindow() override;

        // ------------------------------------------------------------------
        // IWindow interface
        // ------------------------------------------------------------------

        /// Creates (if needed) a minimal wxApp, a wxFrame, and a wxGLCanvas
        /// configured for an OpenGL Core Profile.
        void Init(const WindowConfiguration& config) override;

        /// Dispatches pending wx OS events without
        /// blocking.  This keeps the engine-owned render loop in control.
        void PollEvents() override;

        /// Swaps the front/back buffers of the wxGLCanvas.
        void SwapBuffers() override;

        /// Returns true once shutdown has been requested.
        bool CheckShouldClose() override;

        /// Requests shutdown; destruction waits until GPU resources have been released.
        void Close() override;

        void OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) override;
        void OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) override;
        void OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) override;
        void OnMouseSrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) override;
        void OnWindowReiszeEventCallback(std::function<void(const WindowResizeEventContext&)> callback) override;

        Input::InputState GetInput() const override;
        float             GetCurrentSeconds() override;

        void Accept(IWindowVisitor* visitor) override;

        /// Resets per-frame deltas in both input objects.
        void EndFrame() override;

        // ------------------------------------------------------------------
        // Accessors used by event handlers (defined inline for performance)
        // ------------------------------------------------------------------
        CORE_FORCE_INLINE std::function<void(const WindowMouseMoveEventContext&)>   GetMouseMoveEventCallback()   const { return mMouseMoveCallback;    }
        CORE_FORCE_INLINE std::function<void(const WindowMouseButtonEventContext&)> GetMouseButtonEventCallback() const { return mMouseButtonCallback;   }
        CORE_FORCE_INLINE std::function<void(const WindowKeyboardKeyEventContext&)> GetKeyboardEventCallback()    const { return mKeyboardCallback;      }
        CORE_FORCE_INLINE std::function<void(const WindowMouseScrollEventContext&)> GetMouseScrollEventCallback() const { return mMouseScrollCallback;   }
        CORE_FORCE_INLINE std::function<void(const WindowResizeEventContext&)>      GetWindowResizeEventCallback() const { return mWindowResizeCallback; }

    private:
        friend class WxEngineCanvas;

        // The frame owns its canvas; this wrapper owns the frame and GL context.
        wxFrame*     mFrame     = nullptr;
        wxGLCanvas*  mGLCanvas  = nullptr;
        wxGLContext* mGLContext = nullptr;

        // Input objects
        Unique<Input::Wx::WxKeyboardInput> mKeyboardInput = nullptr;
        Unique<Input::Wx::WxMouseInput>    mMouseInput    = nullptr;

        // Callbacks registered by the engine
        std::function<void(const WindowMouseMoveEventContext&)>   mMouseMoveCallback    = nullptr;
        std::function<void(const WindowMouseButtonEventContext&)> mMouseButtonCallback  = nullptr;
        std::function<void(const WindowKeyboardKeyEventContext&)> mKeyboardCallback     = nullptr;
        std::function<void(const WindowMouseScrollEventContext&)> mMouseScrollCallback  = nullptr;
        std::function<void(const WindowResizeEventContext&)>      mWindowResizeCallback = nullptr;

        bool mShouldClose = false;
        bool mOwnsWxApp = false;
        std::unique_ptr<wxEventLoopBase> mEventLoop;
        std::unique_ptr<wxEventLoopActivator> mEventLoopActivator;

        // High-resolution timer start point
        std::chrono::steady_clock::time_point mStartTime;

        // ------------------------------------------------------------------
        // Helpers
        // ------------------------------------------------------------------
        static WindowMouseButton     _ToWindowMouseButton(int button);
        static WindowMouseButtonState _ToWindowMouseButtonState(bool isDown);
        static WindowKeyboardKey      _ToWindowKeyboardKey(int wxKeyCode);
        static WindowKeyboardKeyState _ToWindowKeyboardKeyState(bool isDown, bool isRepeat);
    };

} // namespace CoreEngine::Window::Wx
