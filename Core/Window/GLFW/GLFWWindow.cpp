#include <glad/glad.h>
#include <Window/IWindowVisitor.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Window::GLFW {
	using namespace CoreEngine::Logger;

	void GLFWWindow::Init(const WindowConfiguration& config) {
		if (!glfwInit()) {
			IP_ENGINE_CRITICAL("Failed to initialize GLFW.");
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		mWindow = glfwCreateWindow(
			config.Width,
			config.Height,
			config.Title.c_str(),
			nullptr,
			nullptr
		);

		if (!mWindow) {
			IP_ENGINE_CRITICAL("Failed to create GLFW window: '{}' ({}x{})", config.Title, config.Width, config.Height);
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(mWindow);

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
			IP_ENGINE_CRITICAL("Failed to initialize OpenGL loader (GLAD).");
			Close();
			return;
		}

		glfwSetWindowUserPointer(mWindow, this);

		IP_ENGINE_TRACE("Window initialized successfully: '{}' ({}x{})", config.Title, config.Width, config.Height);
    
        mKeyboardInput.reset(new Input::GLFW::GLFWKeyBoardInput(mWindow));
        mMouseInput.reset(new Input::GLFW::GLFWMouseInput(mWindow));

        glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xPos, double yPos) {
            GLFWWindow* handler = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
            
            if (handler) {
                static double lastFrameX = xPos;
                static double lastFrameY = yPos;

                double dx = xPos - lastFrameX;
                double dy = yPos - lastFrameY;

                auto mouseInput = handler->GetInput().MouseInput;
                mouseInput->SetDelta(dx, dy);
                mouseInput->SetPosition(xPos, yPos);
                
                lastFrameX = xPos;
                lastFrameY = yPos;

                auto callback = handler->GetMouseMoveEventCallback();
                if (callback) {
                    WindowMouseMoveEventContext evc = { xPos, yPos };
                    callback(evc);
                }
            }
        });

        glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
            GLFWWindow* handler = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

            if (handler) {
                auto callback = handler->GetWindowResizeEventCallback();
                if (callback) {
                    WindowResizeEventContext evc = { width, height };
                    callback(evc);
                }
            }
        });

        glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
            GLFWWindow* handler = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

            if (handler) {
                auto callback = handler->GetMouseButtonEventCallback();
                if (callback) {
                    WindowMouseButton windowMouseButton = GLFWWindow::_ToWindowMouseButton(button);
                    WindowMouseButtonState windowMouseButtonState = GLFWWindow::_ToWindowMouseButtonState(action);
                    WindowMouseButtonEventContext evc = { windowMouseButton, windowMouseButtonState };
                    callback(evc);
                }
            }
        });

        glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            GLFWWindow* handler = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

            if (handler) {
                auto callback = handler->GetKeyboardKeyEventCallback();
                if (callback) {
                    WindowKeyboardKey windowKeyboardKey = GLFWWindow::_ToWindowKeyboardKey(key);
                    WindowKeyboardKeyState windowKeyboardKeyState = GLFWWindow::_ToWindowKeyboardState(action);
                    WindowKeyboardKeyEventContext evc = { windowKeyboardKey, windowKeyboardKeyState };
                    callback(evc);
                }
            }
        });

        glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double dx, double dy) {
            GLFWWindow* handler = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
            
            if (handler) {
                auto mouseInput = handler->GetInput().MouseInput;
                mouseInput->SetScroll(dx, dy);

                auto callback = handler->GetMouseScrollEventCallback();
                if (callback) {
                    callback({ dx,dy });
                }
            }
        });
    }

	void GLFWWindow::PollEvents() {
		glfwPollEvents();
	}

	GLFWwindow* GLFWWindow::GetNativeWindow() {
		return mWindow;
	}

	void GLFWWindow::SwapBuffers() {
		if (mWindow) {
			glfwSwapBuffers(mWindow);
		}
	}

	bool GLFWWindow::CheckShouldClose() {
		return mWindow ? glfwWindowShouldClose(mWindow) : true;
	}

	void GLFWWindow::Close() {
		if (mWindow) {
			glfwDestroyWindow(mWindow);
			mWindow = nullptr;
			glfwTerminate();
			IP_ENGINE_TRACE("Window closed and GLFW terminated.");
		}
	}

	void GLFWWindow::OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) {
		mMouseMoveEventCallback = callback;
	}

	void GLFWWindow::OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) {
		mMouseButtonCallback = callback;
	}

	void GLFWWindow::OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) {
		mWindowKeyboardKeyCallback = callback;
	}

    void GLFWWindow::OnMouseSrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) {
        mMouseScrollCallback = callback;
    }

    void GLFWWindow::OnWindowReiszeEventCallback(std::function<void(const WindowResizeEventContext&)> callback) {
        mWindowResizeEventContext = callback;
    }

    Input::InputState GLFWWindow::GetInput() const {
        return { mKeyboardInput.get(), mMouseInput.get() };
    }

    float GLFWWindow::GetCurrentSeconds() {
        return glfwGetTime();
    }

	void GLFWWindow::Accept(IWindowVisitor* visitor) {
		visitor->VisitGlfwWindow(this);
	}

	void GLFWWindow::EndFrame() {
		mKeyboardInput->Update();
		mMouseInput->Update();
	}

	WindowMouseButton GLFWWindow::_ToWindowMouseButton(int button) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_1: return WindowMouseButton::Button1;
		case GLFW_MOUSE_BUTTON_2: return WindowMouseButton::Button2;
		case GLFW_MOUSE_BUTTON_3: return WindowMouseButton::Button3;
		case GLFW_MOUSE_BUTTON_4: return WindowMouseButton::Button4;
		case GLFW_MOUSE_BUTTON_5: return WindowMouseButton::Button5;
		case GLFW_MOUSE_BUTTON_6: return WindowMouseButton::Button6;
		case GLFW_MOUSE_BUTTON_7: return WindowMouseButton::Button7;
		case GLFW_MOUSE_BUTTON_8: return WindowMouseButton::Button8;
		default: return WindowMouseButton::ButtonLast;
		}
	}

	WindowMouseButtonState GLFWWindow::_ToWindowMouseButtonState(int action) {
		switch (action) {
		case GLFW_PRESS: return WindowMouseButtonState::Pressed;
		case GLFW_RELEASE: return WindowMouseButtonState::Released;
		default: return WindowMouseButtonState::None;
		}
	}

    WindowKeyboardKey GLFWWindow::_ToWindowKeyboardKey(int key) {
        switch (key) {
            // Special Keys and notations
        case GLFW_KEY_UNKNOWN:          return WindowKeyboardKey::Unknow;
        case GLFW_KEY_SPACE:            return WindowKeyboardKey::Space;
        case GLFW_KEY_APOSTROPHE:       return WindowKeyboardKey::Apostrophe;
        case GLFW_KEY_COMMA:            return WindowKeyboardKey::Comma;
        case GLFW_KEY_MINUS:            return WindowKeyboardKey::Minus;
        case GLFW_KEY_PERIOD:           return WindowKeyboardKey::Period;
        case GLFW_KEY_SLASH:            return WindowKeyboardKey::Slash;
        case GLFW_KEY_SEMICOLON:        return WindowKeyboardKey::Semicolon;
        case GLFW_KEY_EQUAL:            return WindowKeyboardKey::Equal;
        case GLFW_KEY_LEFT_BRACKET:     return WindowKeyboardKey::LeftBracket;
        case GLFW_KEY_BACKSLASH:        return WindowKeyboardKey::Backslash;
        case GLFW_KEY_RIGHT_BRACKET:    return WindowKeyboardKey::RightBracket;
        case GLFW_KEY_GRAVE_ACCENT:     return WindowKeyboardKey::GraveAccent;
        case GLFW_KEY_WORLD_1:          return WindowKeyboardKey::World1;
        case GLFW_KEY_WORLD_2:          return WindowKeyboardKey::World2;

            // Numbers in the upper character parts
        case GLFW_KEY_0:                return WindowKeyboardKey::Zero;
        case GLFW_KEY_1:                return WindowKeyboardKey::One;
        case GLFW_KEY_2:                return WindowKeyboardKey::Two;
        case GLFW_KEY_3:                return WindowKeyboardKey::Three;
        case GLFW_KEY_4:                return WindowKeyboardKey::Four;
        case GLFW_KEY_5:                return WindowKeyboardKey::Five;
        case GLFW_KEY_6:                return WindowKeyboardKey::Six;
        case GLFW_KEY_7:                return WindowKeyboardKey::Seven;
        case GLFW_KEY_8:                return WindowKeyboardKey::Eight;
        case GLFW_KEY_9:                return WindowKeyboardKey::Nine;

            // Characters
        case GLFW_KEY_A:                return WindowKeyboardKey::A;
        case GLFW_KEY_B:                return WindowKeyboardKey::B;
        case GLFW_KEY_C:                return WindowKeyboardKey::C;
        case GLFW_KEY_D:                return WindowKeyboardKey::D;
        case GLFW_KEY_E:                return WindowKeyboardKey::E;
        case GLFW_KEY_F:                return WindowKeyboardKey::F;
        case GLFW_KEY_G:                return WindowKeyboardKey::G;
        case GLFW_KEY_H:                return WindowKeyboardKey::H;
        case GLFW_KEY_I:                return WindowKeyboardKey::I;
        case GLFW_KEY_J:                return WindowKeyboardKey::J;
        case GLFW_KEY_K:                return WindowKeyboardKey::K;
        case GLFW_KEY_L:                return WindowKeyboardKey::L;
        case GLFW_KEY_M:                return WindowKeyboardKey::M;
        case GLFW_KEY_N:                return WindowKeyboardKey::N;
        case GLFW_KEY_O:                return WindowKeyboardKey::O;
        case GLFW_KEY_P:                return WindowKeyboardKey::P;
        case GLFW_KEY_Q:                return WindowKeyboardKey::Q;
        case GLFW_KEY_R:                return WindowKeyboardKey::R;
        case GLFW_KEY_S:                return WindowKeyboardKey::S;
        case GLFW_KEY_T:                return WindowKeyboardKey::T;
        case GLFW_KEY_U:                return WindowKeyboardKey::U;
        case GLFW_KEY_V:                return WindowKeyboardKey::V;
        case GLFW_KEY_W:                return WindowKeyboardKey::W;
        case GLFW_KEY_X:                return WindowKeyboardKey::X;
        case GLFW_KEY_Y:                return WindowKeyboardKey::Y;
        case GLFW_KEY_Z:                return WindowKeyboardKey::Z;

            // Functional / Navigation keys
        case GLFW_KEY_ESCAPE:           return WindowKeyboardKey::Escape;
        case GLFW_KEY_ENTER:            return WindowKeyboardKey::Enter;
        case GLFW_KEY_TAB:              return WindowKeyboardKey::Tab;
        case GLFW_KEY_BACKSPACE:        return WindowKeyboardKey::Backspace;
        case GLFW_KEY_INSERT:           return WindowKeyboardKey::Insert;
        case GLFW_KEY_DELETE:           return WindowKeyboardKey::Delete;
        case GLFW_KEY_RIGHT:            return WindowKeyboardKey::Right;
        case GLFW_KEY_LEFT:             return WindowKeyboardKey::Left;
        case GLFW_KEY_DOWN:             return WindowKeyboardKey::Down;
        case GLFW_KEY_UP:               return WindowKeyboardKey::Up;
        case GLFW_KEY_PAGE_UP:          return WindowKeyboardKey::PageUp;
        case GLFW_KEY_PAGE_DOWN:        return WindowKeyboardKey::PageDown;
        case GLFW_KEY_HOME:             return WindowKeyboardKey::Home;
        case GLFW_KEY_END:              return WindowKeyboardKey::End;
        case GLFW_KEY_CAPS_LOCK:        return WindowKeyboardKey::CapsLock;
        case GLFW_KEY_SCROLL_LOCK:      return WindowKeyboardKey::ScrollLock;
        case GLFW_KEY_NUM_LOCK:         return WindowKeyboardKey::NumLock;
        case GLFW_KEY_PRINT_SCREEN:     return WindowKeyboardKey::PrintScreen;
        case GLFW_KEY_PAUSE:            return WindowKeyboardKey::Pause;

            // F-keys
        case GLFW_KEY_F1:               return WindowKeyboardKey::F1;
        case GLFW_KEY_F2:               return WindowKeyboardKey::F2;
        case GLFW_KEY_F3:               return WindowKeyboardKey::F3;
        case GLFW_KEY_F4:               return WindowKeyboardKey::F4;
        case GLFW_KEY_F5:               return WindowKeyboardKey::F5;
        case GLFW_KEY_F6:               return WindowKeyboardKey::F6;
        case GLFW_KEY_F7:               return WindowKeyboardKey::F7;
        case GLFW_KEY_F8:               return WindowKeyboardKey::F8;
        case GLFW_KEY_F9:               return WindowKeyboardKey::F9;
        case GLFW_KEY_F10:              return WindowKeyboardKey::F10;
        case GLFW_KEY_F11:              return WindowKeyboardKey::F11;
        case GLFW_KEY_F12:              return WindowKeyboardKey::F12;
        case GLFW_KEY_F13:              return WindowKeyboardKey::F13;
        case GLFW_KEY_F14:              return WindowKeyboardKey::F14;
        case GLFW_KEY_F15:              return WindowKeyboardKey::F15;
        case GLFW_KEY_F16:              return WindowKeyboardKey::F16;
        case GLFW_KEY_F17:              return WindowKeyboardKey::F17;
        case GLFW_KEY_F18:              return WindowKeyboardKey::F18;
        case GLFW_KEY_F19:              return WindowKeyboardKey::F19;
        case GLFW_KEY_F20:              return WindowKeyboardKey::F20;
        case GLFW_KEY_F21:              return WindowKeyboardKey::F21;
        case GLFW_KEY_F22:              return WindowKeyboardKey::F22;
        case GLFW_KEY_F23:              return WindowKeyboardKey::F23;
        case GLFW_KEY_F24:              return WindowKeyboardKey::F24;
        case GLFW_KEY_F25:              return WindowKeyboardKey::F25;

            // Numpad
        case GLFW_KEY_KP_0:             return WindowKeyboardKey::Numpad0;
        case GLFW_KEY_KP_1:             return WindowKeyboardKey::Numpad1;
        case GLFW_KEY_KP_2:             return WindowKeyboardKey::Numpad2;
        case GLFW_KEY_KP_3:             return WindowKeyboardKey::Numpad3;
        case GLFW_KEY_KP_4:             return WindowKeyboardKey::Numpad4;
        case GLFW_KEY_KP_5:             return WindowKeyboardKey::Numpad5;
        case GLFW_KEY_KP_6:             return WindowKeyboardKey::Numpad6;
        case GLFW_KEY_KP_7:             return WindowKeyboardKey::Numpad7;
        case GLFW_KEY_KP_8:             return WindowKeyboardKey::Numpad8;
        case GLFW_KEY_KP_9:             return WindowKeyboardKey::Numpad9;
        case GLFW_KEY_KP_DECIMAL:       return WindowKeyboardKey::NumpadDecimal;
        case GLFW_KEY_KP_DIVIDE:        return WindowKeyboardKey::NumpadDivide;
        case GLFW_KEY_KP_MULTIPLY:      return WindowKeyboardKey::NumpadMultiply;
        case GLFW_KEY_KP_SUBTRACT:      return WindowKeyboardKey::NumpadSubtract;
        case GLFW_KEY_KP_ADD:           return WindowKeyboardKey::NumpadAdd;
        case GLFW_KEY_KP_ENTER:         return WindowKeyboardKey::NumpadEnter;
        case GLFW_KEY_KP_EQUAL:         return WindowKeyboardKey::NumpadEqual;

            // Modifiers
        case GLFW_KEY_LEFT_SHIFT:       return WindowKeyboardKey::LeftShift;
        case GLFW_KEY_LEFT_CONTROL:     return WindowKeyboardKey::LeftControl;
        case GLFW_KEY_LEFT_ALT:         return WindowKeyboardKey::LeftAlt;
        case GLFW_KEY_LEFT_SUPER:       return WindowKeyboardKey::LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT:      return WindowKeyboardKey::RightShift;
        case GLFW_KEY_RIGHT_CONTROL:    return WindowKeyboardKey::RightControl;
        case GLFW_KEY_RIGHT_ALT:        return WindowKeyboardKey::RightAlt;
        case GLFW_KEY_RIGHT_SUPER:      return WindowKeyboardKey::RightSuper;
        case GLFW_KEY_MENU:             return WindowKeyboardKey::Menu;

        default:                        return WindowKeyboardKey::Unknow;
        }
    }

	WindowKeyboardKeyState GLFWWindow::_ToWindowKeyboardState(int action) {
        switch (action) {
        case GLFW_PRESS: return WindowKeyboardKeyState::Pressed;
        case GLFW_RELEASE: return WindowKeyboardKeyState::Released;
        case GLFW_REPEAT: return WindowKeyboardKeyState::Held;
        default: return WindowKeyboardKeyState::None;
        }
	}
}