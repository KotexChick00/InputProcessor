#include <Window/WindowPlatformFactory.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Logger/LoggerFactory.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Application/Application.hpp>
#include <Input/GLFW/GLFWKeyBoardInput.hpp>
#include <Input/GLFW/GLFWMouseInput.hpp>

namespace CoreEngine {
	Application::Application(ApplicationConfiguration& configuration) : mConfig(configuration) { }

	bool Application::Init() {
		SetupLogger();
		SetupWindow();
		SetupEvents();
		SetupRenderer();
		return mWindow != nullptr;
	}

	void Application::Run() {
		if (!Init()) CORE_ASSERT("Failed to Init Application");
		OnInitClient();

		while (!mWindow->CheckShouldClose()) {
			BeginLoop();
			OnLoopClient();
			EndLoop();
		}

		OnShutdownClient();
		Shutdown();
	}

	void Application::BeginLoop() {
		Time::Time* time = GetTime();
		float minDeltaTime = 1.f / mConfig.MaxFPS;
		float prevFrameTime = time->GetTimeNow();

		while (mWindow->GetCurrentSeconds() - prevFrameTime < minDeltaTime);
		float currentTime = mWindow->GetCurrentSeconds();
		time->SetDeltaTime(currentTime - prevFrameTime);
		time->SetTimeNow(currentTime);

		mWindow->PollEvents();
	}

	void Application::EndLoop() {
        if (auto* GLFWwindow = dynamic_cast<Window::GLFW::GLFWWindow*>(mWindow.get())) {
            GLFWwindow->EndFrame();
        }
		mWindow->SwapBuffers();
	}

	void Application::Shutdown() {
		mWindow->Close();
	}

	void Application::SetupWindow() {
		if (mConfig.WindowPlatformSpec == WindowPlatformSpec::Injection) {
			mWindow.reset(mConfig.InjectedWindow);
		}
		else {
			Window::IWindow* window = nullptr;
			switch (mConfig.WindowPlatformSpec) {
			case WindowPlatformSpec::GLFW:
				window = Window::WindowPlatformFactory::Create(Window::WindowPlatformSpec::GLFW);
				break;
			case WindowPlatformSpec::SDL:
				window = Window::WindowPlatformFactory::Create(Window::WindowPlatformSpec::SDL);
				break;
			default:
				CORE_ASSERT("Currently Window platform spec not found");
			}
			mWindow.reset(window);
		}

		Window::WindowConfiguration windowConfig;
		windowConfig.Width = mConfig.Width;
		windowConfig.Height = mConfig.Height;
		windowConfig.Title = mConfig.Title;

		mWindow->Init(windowConfig);
	}

	void Application::SetupLogger() {
		Logger::Logger::SetClientImplementation(Logger::LoggerFactory::Create(Logger::LoggerSpec::SpdLog, "CLIENT"));
		Logger::Logger::SetEngineImplementation(Logger::LoggerFactory::Create(Logger::LoggerSpec::SpdLog, "CORE_ENGINE"));
	}

	void Application::SetupRenderer() {
		if (mConfig.RenderAPI == RenderAPI::Injection) {
			mRenderer.reset(mConfig.InjectedRenderer);
		}
		else {
			Renderer::RendererSpec spec = Renderer::RendererSpec::Opengl;;
			switch (mConfig.RenderAPI) {
			case RenderAPI::Opengl: 
				spec = Renderer::RendererSpec::Opengl;
				break;
			case RenderAPI::DirectX:
				spec = Renderer::RendererSpec::DirectX;
				break;
			case RenderAPI::Vulkan:
				spec = Renderer::RendererSpec::Vulkan;
				break;
			default:
				CORE_ASSERT("Can't find the renderer API match");
				break;
			}

			mRenderer.reset(Renderer::RendererFactory::Create(spec));
		}
        
        auto config = mRenderer->GetConfig();
        config.ViewPortOptions.X = 0;
        config.ViewPortOptions.Y = 0;
        config.ViewPortOptions.Width = mConfig.Width;
        config.ViewPortOptions.Height = mConfig.Height;
        mRenderer->Config(config);
	}

	void Application::SetupEvents() {
		mWindow->OnWindowReiszeEventCallback([this](CoreEngine::Window::WindowResizeEventContext ec) {
			WindowResizeEventContext evc(ec.Width, ec.Height);
			this->mEventDispatcher.Notify(evc);
			});

		mWindow->OnKeyboardEventCallback([this](CoreEngine::Window::WindowKeyboardKeyEventContext ec) {
            KeyBoardKeyEventContext evc(ToApplicationKeyboardKey(ec.Key), ToApplicationKeyboardKeyState(ec.State));
            this->mEventDispatcher.Notify(evc);
			});

        mWindow->OnMouseButtonEventCallback([this](CoreEngine::Window::WindowMouseButtonEventContext ec) {
            MouseButtonEventContext evc(ToApplicationMouseButton(ec.Button), ToApplicationMouseButtonState(ec.State));
            this->mEventDispatcher.Notify(evc);
            });

        mWindow->OnMouseSrollEventCallback([this](CoreEngine::Window::WindowMouseScrollEventContext ec) {
            MouseScrollEventContext evc(ec.OffsetX, ec.OffsetY);
            this->mEventDispatcher.Notify(evc);
            });
	}

    KeyboardKey Application::ToApplicationKeyboardKey(Window::WindowKeyboardKey key) {
        switch (key) {
        case Window::WindowKeyboardKey::Unknow:             return KeyboardKey::Unknow;
        case Window::WindowKeyboardKey::Space:              return KeyboardKey::Space;
        case Window::WindowKeyboardKey::Apostrophe:         return KeyboardKey::Apostrophe;
        case Window::WindowKeyboardKey::Comma:              return KeyboardKey::Comma;
        case Window::WindowKeyboardKey::Minus:              return KeyboardKey::Minus;
        case Window::WindowKeyboardKey::Period:             return KeyboardKey::Period;
        case Window::WindowKeyboardKey::Slash:              return KeyboardKey::Slash;
        case Window::WindowKeyboardKey::Semicolon:          return KeyboardKey::Semicolon;
        case Window::WindowKeyboardKey::Equal:              return KeyboardKey::Equal;
        case Window::WindowKeyboardKey::LeftBracket:        return KeyboardKey::LeftBracket;
        case Window::WindowKeyboardKey::Backslash:          return KeyboardKey::Backslash;
        case Window::WindowKeyboardKey::RightBracket:       return KeyboardKey::RightBracket;
        case Window::WindowKeyboardKey::GraveAccent:        return KeyboardKey::GraveAccent;
        case Window::WindowKeyboardKey::World1:             return KeyboardKey::World1;
        case Window::WindowKeyboardKey::World2:             return KeyboardKey::World2;

        case Window::WindowKeyboardKey::Zero:               return KeyboardKey::Zero;
        case Window::WindowKeyboardKey::One:                return KeyboardKey::One;
        case Window::WindowKeyboardKey::Two:                return KeyboardKey::Two;
        case Window::WindowKeyboardKey::Three:              return KeyboardKey::Three;
        case Window::WindowKeyboardKey::Four:               return KeyboardKey::Four;
        case Window::WindowKeyboardKey::Five:               return KeyboardKey::Five;
        case Window::WindowKeyboardKey::Six:                return KeyboardKey::Six;
        case Window::WindowKeyboardKey::Seven:              return KeyboardKey::Seven;
        case Window::WindowKeyboardKey::Eight:              return KeyboardKey::Eight;
        case Window::WindowKeyboardKey::Nine:               return KeyboardKey::Nine;

        case Window::WindowKeyboardKey::A:                  return KeyboardKey::A;
        case Window::WindowKeyboardKey::B:                  return KeyboardKey::B;
        case Window::WindowKeyboardKey::C:                  return KeyboardKey::C;
        case Window::WindowKeyboardKey::D:                  return KeyboardKey::D;
        case Window::WindowKeyboardKey::E:                  return KeyboardKey::E;
        case Window::WindowKeyboardKey::F:                  return KeyboardKey::F;
        case Window::WindowKeyboardKey::G:                  return KeyboardKey::G;
        case Window::WindowKeyboardKey::H:                  return KeyboardKey::H;
        case Window::WindowKeyboardKey::I:                  return KeyboardKey::I;
        case Window::WindowKeyboardKey::J:                  return KeyboardKey::J;
        case Window::WindowKeyboardKey::K:                  return KeyboardKey::K;
        case Window::WindowKeyboardKey::L:                  return KeyboardKey::L;
        case Window::WindowKeyboardKey::M:                  return KeyboardKey::M;
        case Window::WindowKeyboardKey::N:                  return KeyboardKey::N;
        case Window::WindowKeyboardKey::O:                  return KeyboardKey::O;
        case Window::WindowKeyboardKey::P:                  return KeyboardKey::P;
        case Window::WindowKeyboardKey::Q:                  return KeyboardKey::Q;
        case Window::WindowKeyboardKey::R:                  return KeyboardKey::R;
        case Window::WindowKeyboardKey::S:                  return KeyboardKey::S;
        case Window::WindowKeyboardKey::T:                  return KeyboardKey::T;
        case Window::WindowKeyboardKey::U:                  return KeyboardKey::U;
        case Window::WindowKeyboardKey::V:                  return KeyboardKey::V;
        case Window::WindowKeyboardKey::W:                  return KeyboardKey::W;
        case Window::WindowKeyboardKey::X:                  return KeyboardKey::X;
        case Window::WindowKeyboardKey::Y:                  return KeyboardKey::Y;
        case Window::WindowKeyboardKey::Z:                  return KeyboardKey::Z;

        case Window::WindowKeyboardKey::Escape:             return KeyboardKey::Escape;
        case Window::WindowKeyboardKey::Enter:              return KeyboardKey::Enter;
        case Window::WindowKeyboardKey::Tab:                return KeyboardKey::Tab;
        case Window::WindowKeyboardKey::Backspace:          return KeyboardKey::Backspace;
        case Window::WindowKeyboardKey::Insert:             return KeyboardKey::Insert;
        case Window::WindowKeyboardKey::Delete:             return KeyboardKey::Delete;
        case Window::WindowKeyboardKey::Right:              return KeyboardKey::Right;
        case Window::WindowKeyboardKey::Left:               return KeyboardKey::Left;
        case Window::WindowKeyboardKey::Down:               return KeyboardKey::Down;
        case Window::WindowKeyboardKey::Up:                 return KeyboardKey::Up;
        case Window::WindowKeyboardKey::PageUp:             return KeyboardKey::PageUp;
        case Window::WindowKeyboardKey::PageDown:           return KeyboardKey::PageDown;
        case Window::WindowKeyboardKey::Home:               return KeyboardKey::Home;
        case Window::WindowKeyboardKey::End:                return KeyboardKey::End;
        case Window::WindowKeyboardKey::CapsLock:           return KeyboardKey::CapsLock;
        case Window::WindowKeyboardKey::ScrollLock:         return KeyboardKey::ScrollLock;
        case Window::WindowKeyboardKey::NumLock:            return KeyboardKey::NumLock;
        case Window::WindowKeyboardKey::PrintScreen:        return KeyboardKey::PrintScreen;
        case Window::WindowKeyboardKey::Pause:              return KeyboardKey::Pause;

        case Window::WindowKeyboardKey::F1:                 return KeyboardKey::F1;
        case Window::WindowKeyboardKey::F2:                 return KeyboardKey::F2;
        case Window::WindowKeyboardKey::F3:                 return KeyboardKey::F3;
        case Window::WindowKeyboardKey::F4:                 return KeyboardKey::F4;
        case Window::WindowKeyboardKey::F5:                 return KeyboardKey::F5;
        case Window::WindowKeyboardKey::F6:                 return KeyboardKey::F6;
        case Window::WindowKeyboardKey::F7:                 return KeyboardKey::F7;
        case Window::WindowKeyboardKey::F8:                 return KeyboardKey::F8;
        case Window::WindowKeyboardKey::F9:                 return KeyboardKey::F9;
        case Window::WindowKeyboardKey::F10:                return KeyboardKey::F10;
        case Window::WindowKeyboardKey::F11:                return KeyboardKey::F11;
        case Window::WindowKeyboardKey::F12:                return KeyboardKey::F12;
        case Window::WindowKeyboardKey::F13:                return KeyboardKey::F13;
        case Window::WindowKeyboardKey::F14:                return KeyboardKey::F14;
        case Window::WindowKeyboardKey::F15:                return KeyboardKey::F15;
        case Window::WindowKeyboardKey::F16:                return KeyboardKey::F16;
        case Window::WindowKeyboardKey::F17:                return KeyboardKey::F17;
        case Window::WindowKeyboardKey::F18:                return KeyboardKey::F18;
        case Window::WindowKeyboardKey::F19:                return KeyboardKey::F19;
        case Window::WindowKeyboardKey::F20:                return KeyboardKey::F20;
        case Window::WindowKeyboardKey::F21:                return KeyboardKey::F21;
        case Window::WindowKeyboardKey::F22:                return KeyboardKey::F22;
        case Window::WindowKeyboardKey::F23:                return KeyboardKey::F23;
        case Window::WindowKeyboardKey::F24:                return KeyboardKey::F24;
        case Window::WindowKeyboardKey::F25:                return KeyboardKey::F25;

        case Window::WindowKeyboardKey::Numpad0:            return KeyboardKey::Numpad0;
        case Window::WindowKeyboardKey::Numpad1:            return KeyboardKey::Numpad1;
        case Window::WindowKeyboardKey::Numpad2:            return KeyboardKey::Numpad2;
        case Window::WindowKeyboardKey::Numpad3:            return KeyboardKey::Numpad3;
        case Window::WindowKeyboardKey::Numpad4:            return KeyboardKey::Numpad4;
        case Window::WindowKeyboardKey::Numpad5:            return KeyboardKey::Numpad5;
        case Window::WindowKeyboardKey::Numpad6:            return KeyboardKey::Numpad6;
        case Window::WindowKeyboardKey::Numpad7:            return KeyboardKey::Numpad7;
        case Window::WindowKeyboardKey::Numpad8:            return KeyboardKey::Numpad8;
        case Window::WindowKeyboardKey::Numpad9:            return KeyboardKey::Numpad9;
        case Window::WindowKeyboardKey::NumpadDecimal:      return KeyboardKey::NumpadDecimal;
        case Window::WindowKeyboardKey::NumpadDivide:       return KeyboardKey::NumpadDivide;
        case Window::WindowKeyboardKey::NumpadMultiply:     return KeyboardKey::NumpadMultiply;
        case Window::WindowKeyboardKey::NumpadSubtract:     return KeyboardKey::NumpadSubtract;
        case Window::WindowKeyboardKey::NumpadAdd:          return KeyboardKey::NumpadAdd;
        case Window::WindowKeyboardKey::NumpadEnter:        return KeyboardKey::NumpadEnter;
        case Window::WindowKeyboardKey::NumpadEqual:        return KeyboardKey::NumpadEqual;

        case Window::WindowKeyboardKey::LeftShift:          return KeyboardKey::LeftShift;
        case Window::WindowKeyboardKey::LeftControl:        return KeyboardKey::LeftControl;
        case Window::WindowKeyboardKey::LeftAlt:            return KeyboardKey::LeftAlt;
        case Window::WindowKeyboardKey::LeftSuper:          return KeyboardKey::LeftSuper;
        case Window::WindowKeyboardKey::RightShift:         return KeyboardKey::RightShift;
        case Window::WindowKeyboardKey::RightControl:       return KeyboardKey::RightControl;
        case Window::WindowKeyboardKey::RightAlt:           return KeyboardKey::RightAlt;
        case Window::WindowKeyboardKey::RightSuper:         return KeyboardKey::RightSuper;
        case Window::WindowKeyboardKey::Menu:               return KeyboardKey::Menu;

        default:                                            return KeyboardKey::Unknow;
        }
    }

	KeyboardKeyState Application::ToApplicationKeyboardKeyState(Window::WindowKeyboardKeyState keyState) {
        switch (keyState) {
        case Window::WindowKeyboardKeyState::None:          return KeyboardKeyState::None;
        case Window::WindowKeyboardKeyState::Pressed:       return KeyboardKeyState::Pressed;
        case Window::WindowKeyboardKeyState::Held:          return KeyboardKeyState::Held;
        case Window::WindowKeyboardKeyState::Released:      return KeyboardKeyState::Released;

        default:                                            return KeyboardKeyState::None;
        }
	}

    MouseButton Application::ToApplicationMouseButton(Window::WindowMouseButton mouseButton) {
        switch (mouseButton) {
        case Window::WindowMouseButton::Button1:            return MouseButton::Button1;
        case Window::WindowMouseButton::Button2:            return MouseButton::Button2;
        case Window::WindowMouseButton::Button3:            return MouseButton::Button3;
        case Window::WindowMouseButton::Button4:            return MouseButton::Button4;
        case Window::WindowMouseButton::Button5:            return MouseButton::Button5;
        case Window::WindowMouseButton::Button6:            return MouseButton::Button6;
        case Window::WindowMouseButton::Button7:            return MouseButton::Button7;
        case Window::WindowMouseButton::Button8:            return MouseButton::Button8;

        default:                                            return MouseButton::Button1;
        }
    }

	MouseButtonState Application::ToApplicationMouseButtonState(Window::WindowMouseButtonState mouseButtonState) {
        switch (mouseButtonState) {
        case Window::WindowMouseButtonState::None:          return MouseButtonState::None;
        case Window::WindowMouseButtonState::Pressed:       return MouseButtonState::Pressed;
        case Window::WindowMouseButtonState::Held:          return MouseButtonState::Held;
        case Window::WindowMouseButtonState::Released:      return MouseButtonState::Released;

        default:                                            return MouseButtonState::None;
        }
	}
}