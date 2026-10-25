#include <glad/glad.h>
#include <Window/IWindowVisitor.hpp>
#include <Window/Wx/WxWindow.hpp>
#include <Logger/Logger.hpp>

// wxWidgets — must be included before any std headers that conflict
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/evtloop.h>
#include <stdexcept>

namespace CoreEngine::Window::Wx {
    using namespace CoreEngine::Logger;

    // =========================================================================
    // Internal wxFrame subclass
    //
    // Hosts the wxGLCanvas.  The WxWindow pointer is kept so event handlers
    // can reach back into the engine's callback / input objects without using
    // wxGetApp() or a global.
    // =========================================================================
    class WxEngineFrame : public wxFrame {
    public:
        WxEngineFrame(WxWindow* owner,
                      const wxString& title,
                      wxSize size)
            : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size)
            , mOwner(owner)
        {
            Bind(wxEVT_CLOSE_WINDOW, &WxEngineFrame::OnClose, this);
        }

    private:
        void OnClose(wxCloseEvent& evt) {
            // Signal the engine's render loop
            if (mOwner) mOwner->Close();
            // Preserve the GL context until application shutdown releases GPU resources.
            if (evt.CanVeto()) evt.Veto();
        }

        WxWindow* mOwner = nullptr;
    };

    // =========================================================================
    // Internal wxGLCanvas subclass
    //
    // All input events are forwarded to the owner WxWindow so it can:
    //   a) update the InputState objects, and
    //   b) invoke the engine callbacks.
    // =========================================================================
    class WxEngineCanvas : public wxGLCanvas {
    public:
        WxEngineCanvas(WxWindow* owner, wxWindow* parent, const wxGLAttributes& attrs)
            : wxGLCanvas(parent, attrs)
            , mOwner(owner)
        {
            SetFocus(); // Capture keyboard

            Bind(wxEVT_MOTION,           &WxEngineCanvas::OnMouseMove,   this);
            Bind(wxEVT_LEFT_DOWN,        &WxEngineCanvas::OnMouseDown,   this);
            Bind(wxEVT_LEFT_UP,          &WxEngineCanvas::OnMouseUp,     this);
            Bind(wxEVT_RIGHT_DOWN,       &WxEngineCanvas::OnMouseDown,   this);
            Bind(wxEVT_RIGHT_UP,         &WxEngineCanvas::OnMouseUp,     this);
            Bind(wxEVT_MIDDLE_DOWN,      &WxEngineCanvas::OnMouseDown,   this);
            Bind(wxEVT_MIDDLE_UP,        &WxEngineCanvas::OnMouseUp,     this);
            Bind(wxEVT_AUX1_DOWN,        &WxEngineCanvas::OnMouseDown,   this);
            Bind(wxEVT_AUX1_UP,          &WxEngineCanvas::OnMouseUp,     this);
            Bind(wxEVT_AUX2_DOWN,        &WxEngineCanvas::OnMouseDown,   this);
            Bind(wxEVT_AUX2_UP,          &WxEngineCanvas::OnMouseUp,     this);
            Bind(wxEVT_MOUSEWHEEL,       &WxEngineCanvas::OnMouseWheel,  this);
            Bind(wxEVT_KEY_DOWN,         &WxEngineCanvas::OnKeyDown,     this);
            Bind(wxEVT_KEY_UP,           &WxEngineCanvas::OnKeyUp,       this);
            Bind(wxEVT_PAINT, [this](wxPaintEvent&) { wxPaintDC dc(this); });
            Bind(wxEVT_KILL_FOCUS, &WxEngineCanvas::OnFocusLost, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &WxEngineCanvas::OnCaptureLost, this);
            Bind(wxEVT_SIZE,             &WxEngineCanvas::OnSize,        this);
        }

    private:
        // ------------------------------------------------------------------ //
        // Mouse move
        // ------------------------------------------------------------------ //
        void OnMouseMove(wxMouseEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            double xPos = static_cast<double>(evt.GetX());
            double yPos = static_cast<double>(evt.GetY());

            auto mouseInput = mOwner->GetInput().MouseInput;
            if (mouseInput) {
                float dx = static_cast<float>(xPos - mouseInput->GetPositionX());
                float dy = static_cast<float>(yPos - mouseInput->GetPositionY());
                mouseInput->SetDelta(mouseInput->GetDeltaX() + dx, mouseInput->GetDeltaY() + dy);
                mouseInput->SetPosition(static_cast<float>(xPos),
                                        static_cast<float>(yPos));
            }

            auto cb = mOwner->GetMouseMoveEventCallback();
            if (cb) cb({ xPos, yPos });

            evt.Skip();
        }

        // ------------------------------------------------------------------ //
        // Mouse buttons
        // ------------------------------------------------------------------ //
        void OnMouseDown(wxMouseEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            WindowMouseButton btn   = WxWindow::_ToWindowMouseButton(evt.GetButton());
            WindowMouseButtonState state = WindowMouseButtonState::Pressed;

            SetFocus();
            if (!HasCapture()) CaptureMouse();
            if (auto* mouse = mOwner->GetInput().MouseInput)
                mouse->SetPosition(evt.GetX(), evt.GetY());

            // Notify input object
            Input::MouseButton inputBtn = _ToInputMouseButton(btn);
            auto mouseInput = mOwner->GetInput().MouseInput;
            if (mouseInput) {
                if (!mouseInput->CheckIsPressed(inputBtn))
                    mouseInput->SetDelta(0.0f, 0.0f);
                static_cast<Input::Wx::WxMouseInput*>(mouseInput)->NotifyButtonDown(inputBtn);
            }

            auto cb = mOwner->GetMouseButtonEventCallback();
            if (cb) cb({ btn, state });

            evt.Skip();
        }

        void OnMouseUp(wxMouseEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            WindowMouseButton btn   = WxWindow::_ToWindowMouseButton(evt.GetButton());
            WindowMouseButtonState state = WindowMouseButtonState::Released;

            Input::MouseButton inputBtn = _ToInputMouseButton(btn);
            auto mouseInput = mOwner->GetInput().MouseInput;
            if (mouseInput)
                static_cast<Input::Wx::WxMouseInput*>(mouseInput)->NotifyButtonUp(inputBtn);

            auto cb = mOwner->GetMouseButtonEventCallback();
            if (cb) cb({ btn, state });
            if (!evt.LeftIsDown() && !evt.RightIsDown() && !evt.MiddleIsDown() && HasCapture())
                ReleaseMouse();
            evt.Skip();
        }

        // ------------------------------------------------------------------ //
        // Mouse scroll
        // ------------------------------------------------------------------ //
        void OnMouseWheel(wxMouseEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            // wxMouseEvent::GetWheelAxis(): wxMOUSE_WHEEL_VERTICAL (default) or _HORIZONTAL
            double dx = 0.0, dy = 0.0;
            float delta = static_cast<float>(evt.GetWheelRotation()) /
                          static_cast<float>(evt.GetWheelDelta());

            if (evt.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                dy = delta;
            else
                dx = delta;

            auto mouseInput = mOwner->GetInput().MouseInput;
            if (mouseInput)
                mouseInput->SetScroll(mouseInput->GetScrollX() + static_cast<float>(dx),
                                      mouseInput->GetScrollY() + static_cast<float>(dy));

            auto cb = mOwner->GetMouseScrollEventCallback();
            if (cb) cb({ dx, dy });

            evt.Skip();
        }

        // ------------------------------------------------------------------ //
        // Keyboard
        // ------------------------------------------------------------------ //
        void OnKeyDown(wxKeyEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            int kc = evt.GetKeyCode();

            auto kbInput = mOwner->GetInput().KeyboardInput;
            if (kbInput)
                static_cast<Input::Wx::WxKeyboardInput*>(kbInput)->NotifyKeyDown(kc);

            WindowKeyboardKey      key   = WxWindow::_ToWindowKeyboardKey(kc);
            WindowKeyboardKeyState state = evt.IsAutoRepeat()
                ? WindowKeyboardKeyState::Held
                : WindowKeyboardKeyState::Pressed;

            auto cb = mOwner->GetKeyboardEventCallback();
            if (cb) cb({ key, state });

            evt.Skip();
        }

        void OnKeyUp(wxKeyEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            int kc = evt.GetKeyCode();

            auto kbInput = mOwner->GetInput().KeyboardInput;
            if (kbInput)
                static_cast<Input::Wx::WxKeyboardInput*>(kbInput)->NotifyKeyUp(kc);

            WindowKeyboardKey key = WxWindow::_ToWindowKeyboardKey(kc);
            auto cb = mOwner->GetKeyboardEventCallback();
            if (cb) cb({ key, WindowKeyboardKeyState::Released });

            evt.Skip();
        }

        // ------------------------------------------------------------------ //
        // Resize
        // ------------------------------------------------------------------ //
        void OnSize(wxSizeEvent& evt) {
            if (!mOwner) { evt.Skip(); return; }

            wxSize sz = evt.GetSize();
            auto cb = mOwner->GetWindowResizeEventCallback();
            if (cb)
                cb({ static_cast<unsigned int>(std::max(0, sz.GetWidth()) * GetContentScaleFactor()),
                     static_cast<unsigned int>(std::max(0, sz.GetHeight()) * GetContentScaleFactor()) });

            evt.Skip();
        }

        void ResetInput() {
            if (!mOwner) return;
            auto input = mOwner->GetInput();
            if (input.KeyboardInput) *static_cast<Input::Wx::WxKeyboardInput*>(input.KeyboardInput) = Input::Wx::WxKeyboardInput();
            if (input.MouseInput) *static_cast<Input::Wx::WxMouseInput*>(input.MouseInput) = Input::Wx::WxMouseInput();
        }

        void OnFocusLost(wxFocusEvent& evt) {
            ResetInput();
            if (HasCapture()) ReleaseMouse();
            evt.Skip();
        }

        void OnCaptureLost(wxMouseCaptureLostEvent&) { ResetInput(); }

        // ------------------------------------------------------------------ //
        // Helpers
        // ------------------------------------------------------------------ //
        static Input::MouseButton _ToInputMouseButton(WindowMouseButton btn) {
            switch (btn) {
            case WindowMouseButton::Button1: return Input::MouseButton::Button1;
            case WindowMouseButton::Button2: return Input::MouseButton::Button2;
            case WindowMouseButton::Button3: return Input::MouseButton::Button3;
            case WindowMouseButton::Button4: return Input::MouseButton::Button4;
            case WindowMouseButton::Button5: return Input::MouseButton::Button5;
            default:                         return Input::MouseButton::Button1;
            }
        }

        WxWindow* mOwner = nullptr;
    };

    // =========================================================================
    // Minimal wxApp shim
    //
    // Required by wxWidgets so wxEntryStart() has a valid app object.
    // If the host process already provides a wxApp, Init() skips creation.
    // =========================================================================
    class WxEngineApp : public wxApp {
    public:
        bool OnInit() override { return true; }
    };

    // =========================================================================
    // WxWindow — IWindow implementation
    // =========================================================================

    WxWindow::WxWindow() = default;

    WxWindow::~WxWindow() {
        if (mGLCanvas && mGLCanvas->HasCapture()) mGLCanvas->ReleaseMouse();
        delete mGLContext;
        // No event dispatch is in progress here; the application loop has stopped.
        delete mFrame;
        mEventLoopActivator.reset();
        mEventLoop.reset();
        if (mOwnsWxApp) {
            wxTheApp->OnExit();
            wxEntryCleanup();
        }
    }

    void WxWindow::Init(const WindowConfiguration& config) {
        if (!wxTheApp) {
            wxApp::SetInstance(new WxEngineApp());
            int argc = 0;
            char** argv = nullptr;
            if (!wxEntryStart(argc, argv))
                throw std::runtime_error("Failed to initialize wxWidgets");
            mOwnsWxApp = true;
            if (!wxTheApp->CallOnInit())
                throw std::runtime_error("Failed to initialize wxApp");
        }
        if (!wxEventLoopBase::GetActive()) {
            mEventLoop = std::make_unique<wxEventLoop>();
            mEventLoopActivator = std::make_unique<wxEventLoopActivator>(mEventLoop.get());
        }

        mKeyboardInput = std::make_shared<Input::Wx::WxKeyboardInput>();
        mMouseInput = std::make_shared<Input::Wx::WxMouseInput>();
        mFrame = new WxEngineFrame(this, wxString::FromUTF8(config.Title.c_str()), wxDefaultSize);
        mFrame->SetClientSize(config.Width, config.Height);
        wxGLAttributes display;
        display.PlatformDefaults().RGBA().DoubleBuffer().Depth(24).Stencil(8).EndList();
        if (!wxGLCanvas::IsDisplaySupported(display))
            throw std::runtime_error("Required OpenGL display format is unavailable");
        mGLCanvas = new WxEngineCanvas(this, mFrame, display);
        mGLCanvas->SetSize(mFrame->GetClientSize());
        wxGLContextAttrs context;
        context.PlatformDefaults().CoreProfile();
#ifdef __APPLE__
        context.OGLVersion(4, 1).ForwardCompatible();
#else
        context.OGLVersion(3, 3);
#endif
        context.EndList();
        mGLContext = new wxGLContext(mGLCanvas, nullptr, &context);
        if (!mGLContext->IsOK())
            throw std::runtime_error("Failed to create OpenGL Core context");
        mFrame->Show();
        mGLCanvas->SetFocus();
        if (!mGLCanvas->SetCurrent(*mGLContext) || !gladLoadGL())
            throw std::runtime_error("Failed to load OpenGL functions");
        mStartTime = std::chrono::steady_clock::now();
        IP_ENGINE_TRACE("[WxWindow] Initialized '{}'", config.Title);
    }

    void WxWindow::PollEvents() {
        wxTheApp->ProcessPendingEvents();
        auto* loop = wxEventLoopBase::GetActive();
        while (!mShouldClose && loop && loop->Pending()) loop->Dispatch();
        wxTheApp->ProcessIdle();
        if (mGLCanvas && mGLContext) mGLCanvas->SetCurrent(*mGLContext);
    }

    void WxWindow::SwapBuffers() {
        if (mGLCanvas && mGLContext) {
            mGLCanvas->SwapBuffers();
        }
    }

    // -------------------------------------------------------------------------

    bool WxWindow::CheckShouldClose() {
        return mShouldClose;
    }

    // -------------------------------------------------------------------------

    void WxWindow::Close() {
        mShouldClose = true;
    }

    void WxWindow::OnMouseMoveEventCallback(
        std::function<void(const WindowMouseMoveEventContext&)> callback) {
        mMouseMoveCallback = callback;
    }

    void WxWindow::OnMouseButtonEventCallback(
        std::function<void(const WindowMouseButtonEventContext&)> callback) {
        mMouseButtonCallback = callback;
    }

    void WxWindow::OnKeyboardEventCallback(
        std::function<void(const WindowKeyboardKeyEventContext&)> callback) {
        mKeyboardCallback = callback;
    }

    void WxWindow::OnMouseSrollEventCallback(
        std::function<void(const WindowMouseScrollEventContext&)> callback) {
        mMouseScrollCallback = callback;
    }

    void WxWindow::OnWindowReiszeEventCallback(
        std::function<void(const WindowResizeEventContext&)> callback) {
        mWindowResizeCallback = callback;
        if (mGLCanvas && callback) {
            auto size = mGLCanvas->GetClientSize();
            auto scale = mGLCanvas->GetContentScaleFactor();
            callback({static_cast<unsigned int>(size.x * scale), static_cast<unsigned int>(size.y * scale)});
        }
    }

    // -------------------------------------------------------------------------

    Input::InputState WxWindow::GetInput() const {
        return { mKeyboardInput.get(), mMouseInput.get() };
    }

    // -------------------------------------------------------------------------

    float WxWindow::GetCurrentSeconds() {
        auto now     = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(now - mStartTime);
        return elapsed.count();
    }

    // -------------------------------------------------------------------------

    void WxWindow::Accept(IWindowVisitor* visitor) {
        visitor->VisitWxWindow(this);
    }

    // -------------------------------------------------------------------------

    void WxWindow::EndFrame() {
        if (mKeyboardInput) mKeyboardInput->Update();
        if (mMouseInput)    mMouseInput->Update();
    }

    // =========================================================================
    // Static conversion helpers
    // =========================================================================

    WindowMouseButton WxWindow::_ToWindowMouseButton(int button) {
        switch (button) {
        case wxMOUSE_BTN_LEFT:    return WindowMouseButton::Button1;
        case wxMOUSE_BTN_RIGHT:   return WindowMouseButton::Button2;
        case wxMOUSE_BTN_MIDDLE:  return WindowMouseButton::Button3;
        case wxMOUSE_BTN_AUX1:   return WindowMouseButton::Button4;
        case wxMOUSE_BTN_AUX2:   return WindowMouseButton::Button5;
        default:                  return WindowMouseButton::ButtonLast;
        }
    }

    WindowMouseButtonState WxWindow::_ToWindowMouseButtonState(bool isDown) {
        return isDown ? WindowMouseButtonState::Pressed
                      : WindowMouseButtonState::Released;
    }

    WindowKeyboardKey WxWindow::_ToWindowKeyboardKey(int kc) {
        // Letters (wxWidgets gives uppercase ASCII)
        if (kc >= 'A' && kc <= 'Z') {
            static const WindowKeyboardKey letters[] = {
                WindowKeyboardKey::A, WindowKeyboardKey::B, WindowKeyboardKey::C,
                WindowKeyboardKey::D, WindowKeyboardKey::E, WindowKeyboardKey::F,
                WindowKeyboardKey::G, WindowKeyboardKey::H, WindowKeyboardKey::I,
                WindowKeyboardKey::J, WindowKeyboardKey::K, WindowKeyboardKey::L,
                WindowKeyboardKey::M, WindowKeyboardKey::N, WindowKeyboardKey::O,
                WindowKeyboardKey::P, WindowKeyboardKey::Q, WindowKeyboardKey::R,
                WindowKeyboardKey::S, WindowKeyboardKey::T, WindowKeyboardKey::U,
                WindowKeyboardKey::V, WindowKeyboardKey::W, WindowKeyboardKey::X,
                WindowKeyboardKey::Y, WindowKeyboardKey::Z
            };
            return letters[kc - 'A'];
        }
        if (kc >= '0' && kc <= '9') {
            static const WindowKeyboardKey digits[] = {
                WindowKeyboardKey::Zero,  WindowKeyboardKey::One,   WindowKeyboardKey::Two,
                WindowKeyboardKey::Three, WindowKeyboardKey::Four,  WindowKeyboardKey::Five,
                WindowKeyboardKey::Six,   WindowKeyboardKey::Seven, WindowKeyboardKey::Eight,
                WindowKeyboardKey::Nine
            };
            return digits[kc - '0'];
        }
        if (kc >= WXK_F1 && kc <= WXK_F24) {
            static const WindowKeyboardKey fkeys[] = {
                WindowKeyboardKey::F1,  WindowKeyboardKey::F2,  WindowKeyboardKey::F3,
                WindowKeyboardKey::F4,  WindowKeyboardKey::F5,  WindowKeyboardKey::F6,
                WindowKeyboardKey::F7,  WindowKeyboardKey::F8,  WindowKeyboardKey::F9,
                WindowKeyboardKey::F10, WindowKeyboardKey::F11, WindowKeyboardKey::F12,
                WindowKeyboardKey::F13, WindowKeyboardKey::F14, WindowKeyboardKey::F15,
                WindowKeyboardKey::F16, WindowKeyboardKey::F17, WindowKeyboardKey::F18,
                WindowKeyboardKey::F19, WindowKeyboardKey::F20, WindowKeyboardKey::F21,
                WindowKeyboardKey::F22, WindowKeyboardKey::F23, WindowKeyboardKey::F24
            };
            int idx = kc - WXK_F1;
            if (idx < 24) return fkeys[idx];
        }
        if (kc >= WXK_NUMPAD0 && kc <= WXK_NUMPAD9) {
            static const WindowKeyboardKey np[] = {
                WindowKeyboardKey::Numpad0, WindowKeyboardKey::Numpad1,
                WindowKeyboardKey::Numpad2, WindowKeyboardKey::Numpad3,
                WindowKeyboardKey::Numpad4, WindowKeyboardKey::Numpad5,
                WindowKeyboardKey::Numpad6, WindowKeyboardKey::Numpad7,
                WindowKeyboardKey::Numpad8, WindowKeyboardKey::Numpad9
            };
            return np[kc - WXK_NUMPAD0];
        }

        switch (kc) {
        case WXK_SPACE:           return WindowKeyboardKey::Space;
        case WXK_ESCAPE:          return WindowKeyboardKey::Escape;
        case WXK_RETURN:          return WindowKeyboardKey::Enter;
        case WXK_TAB:             return WindowKeyboardKey::Tab;
        case WXK_BACK:            return WindowKeyboardKey::Backspace;
        case WXK_INSERT:          return WindowKeyboardKey::Insert;
        case WXK_DELETE:          return WindowKeyboardKey::Delete;
        case WXK_RIGHT:           return WindowKeyboardKey::Right;
        case WXK_LEFT:            return WindowKeyboardKey::Left;
        case WXK_DOWN:            return WindowKeyboardKey::Down;
        case WXK_UP:              return WindowKeyboardKey::Up;
        case WXK_PAGEUP:          return WindowKeyboardKey::PageUp;
        case WXK_PAGEDOWN:        return WindowKeyboardKey::PageDown;
        case WXK_HOME:            return WindowKeyboardKey::Home;
        case WXK_END:             return WindowKeyboardKey::End;
        case WXK_CAPITAL:         return WindowKeyboardKey::CapsLock;
        case WXK_SCROLL:          return WindowKeyboardKey::ScrollLock;
        case WXK_NUMLOCK:         return WindowKeyboardKey::NumLock;
        case WXK_PRINT:           return WindowKeyboardKey::PrintScreen;
        case WXK_PAUSE:           return WindowKeyboardKey::Pause;
        case WXK_SHIFT:           return WindowKeyboardKey::LeftShift;
        case WXK_CONTROL:         return WindowKeyboardKey::LeftControl;
        case WXK_ALT:             return WindowKeyboardKey::LeftAlt;
        case WXK_WINDOWS_LEFT:    return WindowKeyboardKey::LeftSuper;
        case WXK_WINDOWS_RIGHT:   return WindowKeyboardKey::RightSuper;
        case WXK_MENU:            return WindowKeyboardKey::Menu;
        case WXK_NUMPAD_DECIMAL:  return WindowKeyboardKey::NumpadDecimal;
        case WXK_NUMPAD_DIVIDE:   return WindowKeyboardKey::NumpadDivide;
        case WXK_NUMPAD_MULTIPLY: return WindowKeyboardKey::NumpadMultiply;
        case WXK_NUMPAD_SUBTRACT: return WindowKeyboardKey::NumpadSubtract;
        case WXK_NUMPAD_ADD:      return WindowKeyboardKey::NumpadAdd;
        case WXK_NUMPAD_ENTER:    return WindowKeyboardKey::NumpadEnter;
        case WXK_NUMPAD_EQUAL:    return WindowKeyboardKey::NumpadEqual;
        // Punctuation (raw ASCII)
        case '\'':  return WindowKeyboardKey::Apostrophe;
        case ',':   return WindowKeyboardKey::Comma;
        case '-':   return WindowKeyboardKey::Minus;
        case '.':   return WindowKeyboardKey::Period;
        case '/':   return WindowKeyboardKey::Slash;
        case ';':   return WindowKeyboardKey::Semicolon;
        case '=':   return WindowKeyboardKey::Equal;
        case '[':   return WindowKeyboardKey::LeftBracket;
        case '\\':  return WindowKeyboardKey::Backslash;
        case ']':   return WindowKeyboardKey::RightBracket;
        case '`':   return WindowKeyboardKey::GraveAccent;
        default:    return WindowKeyboardKey::Unknow;
        }
    }

    WindowKeyboardKeyState WxWindow::_ToWindowKeyboardKeyState(bool isDown, bool isRepeat) {
        if (!isDown)    return WindowKeyboardKeyState::Released;
        if (isRepeat)   return WindowKeyboardKeyState::Held;
        return WindowKeyboardKeyState::Pressed;
    }

} // namespace CoreEngine::Window::Wx
