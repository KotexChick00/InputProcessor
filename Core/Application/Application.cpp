#include <Window/WindowPlatformFactory.hpp>
#include <Logger/LoggerFactory.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Application/Application.hpp>
#include <chrono>
#include <thread>
#include <stdexcept>

namespace CoreEngine
{
    Application::Application(ApplicationConfiguration& configuration)
        : mConfig(configuration)
    {
    }

    bool Application::Init()
    {
        if (!mConfig.Width || !mConfig.Height || !mConfig.MaxFPS || !mConfig.Title)
            throw std::invalid_argument("Invalid application configuration");
        SetupLogger();
        SetupWindow();
        if (!mWindow || mWindow->CheckShouldClose()) return false;
        SetupRenderer();
        SetupEvents();
        GetTime()->SetTimeNow(mWindow->GetCurrentSeconds());
        return mRenderer != nullptr;
    }

    void Application::Run()
    {
        if (!Init()) throw std::runtime_error("Failed to initialize application");
        try
        {
            OnInitClient();
            while (!mCloseRequested && !mWindow->CheckShouldClose())
            {
                BeginLoop();
                if (mCloseRequested || mWindow->CheckShouldClose()) break;
                OnLoopClient();
                EndLoop();
            }
        }
        catch (...)
        {
            OnShutdownClient();
            Shutdown();
            throw;
        }
        OnShutdownClient();
        Shutdown();
    }

    void Application::BeginLoop()
    {
        Time* time = GetTime();
        float minDeltaTime = 1.f / mConfig.MaxFPS;
        float prevFrameTime = time->GetTimeNow();

        // OS sleeps are coarse (~15.6 ms timer resolution by default on Windows), so sleeping for
        // a short remainder overshoots, misses the next v-sync and halves the frame rate.
        // Sleep only while more than one timer tick is left, then yield until the frame deadline.
        constexpr float SleepGranularity = 0.016f;
        const float targetTime = prevFrameTime + minDeltaTime;
        float currentTime = mWindow->GetCurrentSeconds();
        while (targetTime - currentTime > SleepGranularity)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            currentTime = mWindow->GetCurrentSeconds();
        }
        while (currentTime < targetTime)
        {
            std::this_thread::yield();
            currentTime = mWindow->GetCurrentSeconds();
        }
        time->SetDeltaTime(currentTime - prevFrameTime);
        time->SetTimeNow(currentTime);

        mWindow->PollEvents();
    }

    void Application::EndLoop()
    {
        mWindow->EndFrame();
        mWindow->SwapBuffers();
    }

    void Application::Shutdown()
    {
        mRenderer.reset(); // Release GPU resources while the window's context is current.
        if (mWindow) mWindow->Close();
        mWindow.reset();
        mInputState = {};
    }

    void Application::SetupWindow()
    {
        if (mConfig.WindowPlatformSpec == WindowPlatformSpec::Injection)
        {
            mWindow.reset(mConfig.InjectedWindow);
        }
        else
        {
            mWindow.reset(WindowPlatformFactory::Create(mConfig.WindowPlatformSpec));
        }

        WindowConfiguration windowConfig;
        windowConfig.Width = mConfig.Width;
        windowConfig.Height = mConfig.Height;
        windowConfig.Title = mConfig.Title;

        if (mWindow)
        {
            mWindow->Init(windowConfig);
            mInputState = mWindow->GetInput();
        }
    }

    void Application::SetupLogger()
    {
        Logger::SetClientImplementation(LoggerFactory::Create(LoggerSpec::SpdLog, "CLIENT"));
        Logger::SetEngineImplementation(LoggerFactory::Create(LoggerSpec::SpdLog, "CORE_ENGINE"));
    }

    void Application::SetupRenderer()
    {
        if (mConfig.RenderAPI == RenderAPI::Injection)
        {
            mRenderer.reset(mConfig.InjectedRenderer);
        }
        else
        {
            RendererSpec spec = RendererSpec::Opengl;
            ;
            switch (mConfig.RenderAPI)
            {
            case RenderAPI::Opengl:
                spec = RendererSpec::Opengl;
                break;
            case RenderAPI::DirectX:
                spec = RendererSpec::DirectX;
                break;
            case RenderAPI::Vulkan:
                spec = RendererSpec::Vulkan;
                break;
            default:
                CORE_ASSERT("Can't find the renderer API match");
                break;
            }

            mRenderer.reset(RendererFactory::Create(spec));
        }

        if (!mRenderer) throw std::runtime_error("Renderer is unavailable");
        auto config = mRenderer->GetConfig();
        config.ViewPortOptions.X = 0;
        config.ViewPortOptions.Y = 0;
        config.ViewPortOptions.Width = mConfig.Width;
        config.ViewPortOptions.Height = mConfig.Height;
        mRenderer->Config(config);
    }

    void Application::SetupEvents()
    {
        mWindow->OnWindowReiszeEventCallback([this](CoreEngine::WindowSizeEventContext ec) {
            if (mRenderer && ec.Width && ec.Height)
            {
                auto config = mRenderer->GetConfig();
                config.ViewPortOptions.Width = ec.Width;
                config.ViewPortOptions.Height = ec.Height;
                mRenderer->Config(config);
            }
            WindowResizeEventContext evc(ec.Width, ec.Height);
            this->mEventDispatcher.Notify(evc);
        });

        mWindow->OnKeyboardEventCallback([this](CoreEngine::WindowKeyboardKeyEventContext ec) {
            KeyBoardKeyEventContext evc(ToApplicationKeyboardKey(ec.Key), ToApplicationKeyboardKeyState(ec.State));
            this->mEventDispatcher.Notify(evc);
        });

        mWindow->OnMouseButtonEventCallback([this](CoreEngine::WindowMouseButtonEventContext ec) {
            MouseButtonEventContext evc(ToApplicationMouseButton(ec.Button), ToApplicationMouseButtonState(ec.State));
            this->mEventDispatcher.Notify(evc);
        });

        mWindow->OnMouseSrollEventCallback([this](CoreEngine::WindowMouseScrollEventContext ec) {
            MouseScrollEventContext evc(ec.OffsetX, ec.OffsetY);
            this->mEventDispatcher.Notify(evc);
        });
    }

    KeyboardKey Application::ToApplicationKeyboardKey(WindowKeyboardKey key)
    {
        switch (key)
        {
        case WindowKeyboardKey::Unknow:         return KeyboardKey::Unknow;
        case WindowKeyboardKey::Space:          return KeyboardKey::Space;
        case WindowKeyboardKey::Apostrophe:     return KeyboardKey::Apostrophe;
        case WindowKeyboardKey::Comma:          return KeyboardKey::Comma;
        case WindowKeyboardKey::Minus:          return KeyboardKey::Minus;
        case WindowKeyboardKey::Period:         return KeyboardKey::Period;
        case WindowKeyboardKey::Slash:          return KeyboardKey::Slash;
        case WindowKeyboardKey::Semicolon:      return KeyboardKey::Semicolon;
        case WindowKeyboardKey::Equal:          return KeyboardKey::Equal;
        case WindowKeyboardKey::LeftBracket:    return KeyboardKey::LeftBracket;
        case WindowKeyboardKey::Backslash:      return KeyboardKey::Backslash;
        case WindowKeyboardKey::RightBracket:   return KeyboardKey::RightBracket;
        case WindowKeyboardKey::GraveAccent:    return KeyboardKey::GraveAccent;
        case WindowKeyboardKey::World1:         return KeyboardKey::World1;
        case WindowKeyboardKey::World2:         return KeyboardKey::World2;

        case WindowKeyboardKey::Zero:           return KeyboardKey::Zero;
        case WindowKeyboardKey::One:            return KeyboardKey::One;
        case WindowKeyboardKey::Two:            return KeyboardKey::Two;
        case WindowKeyboardKey::Three:          return KeyboardKey::Three;
        case WindowKeyboardKey::Four:           return KeyboardKey::Four;
        case WindowKeyboardKey::Five:           return KeyboardKey::Five;
        case WindowKeyboardKey::Six:            return KeyboardKey::Six;
        case WindowKeyboardKey::Seven:          return KeyboardKey::Seven;
        case WindowKeyboardKey::Eight:          return KeyboardKey::Eight;
        case WindowKeyboardKey::Nine:           return KeyboardKey::Nine;

        case WindowKeyboardKey::A:              return KeyboardKey::A;
        case WindowKeyboardKey::B:              return KeyboardKey::B;
        case WindowKeyboardKey::C:              return KeyboardKey::C;
        case WindowKeyboardKey::D:              return KeyboardKey::D;
        case WindowKeyboardKey::E:              return KeyboardKey::E;
        case WindowKeyboardKey::F:              return KeyboardKey::F;
        case WindowKeyboardKey::G:              return KeyboardKey::G;
        case WindowKeyboardKey::H:              return KeyboardKey::H;
        case WindowKeyboardKey::I:              return KeyboardKey::I;
        case WindowKeyboardKey::J:              return KeyboardKey::J;
        case WindowKeyboardKey::K:              return KeyboardKey::K;
        case WindowKeyboardKey::L:              return KeyboardKey::L;
        case WindowKeyboardKey::M:              return KeyboardKey::M;
        case WindowKeyboardKey::N:              return KeyboardKey::N;
        case WindowKeyboardKey::O:              return KeyboardKey::O;
        case WindowKeyboardKey::P:              return KeyboardKey::P;
        case WindowKeyboardKey::Q:              return KeyboardKey::Q;
        case WindowKeyboardKey::R:              return KeyboardKey::R;
        case WindowKeyboardKey::S:              return KeyboardKey::S;
        case WindowKeyboardKey::T:              return KeyboardKey::T;
        case WindowKeyboardKey::U:              return KeyboardKey::U;
        case WindowKeyboardKey::V:              return KeyboardKey::V;
        case WindowKeyboardKey::W:              return KeyboardKey::W;
        case WindowKeyboardKey::X:              return KeyboardKey::X;
        case WindowKeyboardKey::Y:              return KeyboardKey::Y;
        case WindowKeyboardKey::Z:              return KeyboardKey::Z;

        case WindowKeyboardKey::Escape:         return KeyboardKey::Escape;
        case WindowKeyboardKey::Enter:          return KeyboardKey::Enter;
        case WindowKeyboardKey::Tab:            return KeyboardKey::Tab;
        case WindowKeyboardKey::Backspace:      return KeyboardKey::Backspace;
        case WindowKeyboardKey::Insert:         return KeyboardKey::Insert;
        case WindowKeyboardKey::Delete:         return KeyboardKey::Delete;
        case WindowKeyboardKey::Right:          return KeyboardKey::Right;
        case WindowKeyboardKey::Left:           return KeyboardKey::Left;
        case WindowKeyboardKey::Down:           return KeyboardKey::Down;
        case WindowKeyboardKey::Up:             return KeyboardKey::Up;
        case WindowKeyboardKey::PageUp:         return KeyboardKey::PageUp;
        case WindowKeyboardKey::PageDown:       return KeyboardKey::PageDown;
        case WindowKeyboardKey::Home:           return KeyboardKey::Home;
        case WindowKeyboardKey::End:            return KeyboardKey::End;
        case WindowKeyboardKey::CapsLock:       return KeyboardKey::CapsLock;
        case WindowKeyboardKey::ScrollLock:     return KeyboardKey::ScrollLock;
        case WindowKeyboardKey::NumLock:        return KeyboardKey::NumLock;
        case WindowKeyboardKey::PrintScreen:    return KeyboardKey::PrintScreen;
        case WindowKeyboardKey::Pause:          return KeyboardKey::Pause;

        case WindowKeyboardKey::F1:             return KeyboardKey::F1;
        case WindowKeyboardKey::F2:             return KeyboardKey::F2;
        case WindowKeyboardKey::F3:             return KeyboardKey::F3;
        case WindowKeyboardKey::F4:             return KeyboardKey::F4;
        case WindowKeyboardKey::F5:             return KeyboardKey::F5;
        case WindowKeyboardKey::F6:             return KeyboardKey::F6;
        case WindowKeyboardKey::F7:             return KeyboardKey::F7;
        case WindowKeyboardKey::F8:             return KeyboardKey::F8;
        case WindowKeyboardKey::F9:             return KeyboardKey::F9;
        case WindowKeyboardKey::F10:            return KeyboardKey::F10;
        case WindowKeyboardKey::F11:            return KeyboardKey::F11;
        case WindowKeyboardKey::F12:            return KeyboardKey::F12;
        case WindowKeyboardKey::F13:            return KeyboardKey::F13;
        case WindowKeyboardKey::F14:            return KeyboardKey::F14;
        case WindowKeyboardKey::F15:            return KeyboardKey::F15;
        case WindowKeyboardKey::F16:            return KeyboardKey::F16;
        case WindowKeyboardKey::F17:            return KeyboardKey::F17;
        case WindowKeyboardKey::F18:            return KeyboardKey::F18;
        case WindowKeyboardKey::F19:            return KeyboardKey::F19;
        case WindowKeyboardKey::F20:            return KeyboardKey::F20;
        case WindowKeyboardKey::F21:            return KeyboardKey::F21;
        case WindowKeyboardKey::F22:            return KeyboardKey::F22;
        case WindowKeyboardKey::F23:            return KeyboardKey::F23;
        case WindowKeyboardKey::F24:            return KeyboardKey::F24;
        case WindowKeyboardKey::F25:            return KeyboardKey::F25;

        case WindowKeyboardKey::Numpad0:        return KeyboardKey::Numpad0;
        case WindowKeyboardKey::Numpad1:        return KeyboardKey::Numpad1;
        case WindowKeyboardKey::Numpad2:        return KeyboardKey::Numpad2;
        case WindowKeyboardKey::Numpad3:        return KeyboardKey::Numpad3;
        case WindowKeyboardKey::Numpad4:        return KeyboardKey::Numpad4;
        case WindowKeyboardKey::Numpad5:        return KeyboardKey::Numpad5;
        case WindowKeyboardKey::Numpad6:        return KeyboardKey::Numpad6;
        case WindowKeyboardKey::Numpad7:        return KeyboardKey::Numpad7;
        case WindowKeyboardKey::Numpad8:        return KeyboardKey::Numpad8;
        case WindowKeyboardKey::Numpad9:        return KeyboardKey::Numpad9;
        case WindowKeyboardKey::NumpadDecimal:  return KeyboardKey::NumpadDecimal;
        case WindowKeyboardKey::NumpadDivide:   return KeyboardKey::NumpadDivide;
        case WindowKeyboardKey::NumpadMultiply: return KeyboardKey::NumpadMultiply;
        case WindowKeyboardKey::NumpadSubtract: return KeyboardKey::NumpadSubtract;
        case WindowKeyboardKey::NumpadAdd:      return KeyboardKey::NumpadAdd;
        case WindowKeyboardKey::NumpadEnter:    return KeyboardKey::NumpadEnter;
        case WindowKeyboardKey::NumpadEqual:    return KeyboardKey::NumpadEqual;

        case WindowKeyboardKey::LeftShift:      return KeyboardKey::LeftShift;
        case WindowKeyboardKey::LeftControl:    return KeyboardKey::LeftControl;
        case WindowKeyboardKey::LeftAlt:        return KeyboardKey::LeftAlt;
        case WindowKeyboardKey::LeftSuper:      return KeyboardKey::LeftSuper;
        case WindowKeyboardKey::RightShift:     return KeyboardKey::RightShift;
        case WindowKeyboardKey::RightControl:   return KeyboardKey::RightControl;
        case WindowKeyboardKey::RightAlt:       return KeyboardKey::RightAlt;
        case WindowKeyboardKey::RightSuper:     return KeyboardKey::RightSuper;
        case WindowKeyboardKey::Menu:           return KeyboardKey::Menu;

        default:                                return KeyboardKey::Unknow;
        }
    }

    KeyboardKeyState Application::ToApplicationKeyboardKeyState(WindowKeyboardKeyState keyState)
    {
        switch (keyState)
        {
        case WindowKeyboardKeyState::None:     return KeyboardKeyState::None;
        case WindowKeyboardKeyState::Pressed:  return KeyboardKeyState::Pressed;
        case WindowKeyboardKeyState::Held:     return KeyboardKeyState::Held;
        case WindowKeyboardKeyState::Released: return KeyboardKeyState::Released;

        default:                               return KeyboardKeyState::None;
        }
    }

    MouseButton Application::ToApplicationMouseButton(WindowMouseButton mouseButton)
    {
        switch (mouseButton)
        {
        case WindowMouseButton::Button1: return MouseButton::Button1;
        case WindowMouseButton::Button2: return MouseButton::Button2;
        case WindowMouseButton::Button3: return MouseButton::Button3;
        case WindowMouseButton::Button4: return MouseButton::Button4;
        case WindowMouseButton::Button5: return MouseButton::Button5;
        case WindowMouseButton::Button6: return MouseButton::Button6;
        case WindowMouseButton::Button7: return MouseButton::Button7;
        case WindowMouseButton::Button8: return MouseButton::Button8;

        default:                         return MouseButton::Button1;
        }
    }

    MouseButtonState Application::ToApplicationMouseButtonState(WindowMouseButtonState mouseButtonState)
    {
        switch (mouseButtonState)
        {
        case WindowMouseButtonState::None:     return MouseButtonState::None;
        case WindowMouseButtonState::Pressed:  return MouseButtonState::Pressed;
        case WindowMouseButtonState::Held:     return MouseButtonState::Held;
        case WindowMouseButtonState::Released: return MouseButtonState::Released;

        default:                               return MouseButtonState::None;
        }
    }
}
