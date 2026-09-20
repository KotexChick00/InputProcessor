#pragma once
#include <Window/IWindow.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace CoreEngine::Window::GLFW {
	class GLFWWindow : public IWindow {
	public:
		void Init(const WindowConfiguration& config) override;
		void PollEvents() override;
		void SwapBuffers() override;
		bool CheckShouldClose() override;
		void Close() override;

		void OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) override;
		void OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) override;
		void OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) override;
		void OnMouseSrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) override;
		void OnWindowReiszeEventCallback(std::function<void(const WindowResizeEventContext&)> callback) override;

		float GetCurrentSeconds() override;

		void Accept(IWindowVisitor* visitor) override;

		GLFWwindow* GetNativeWindow();

		void CursorPosCallback(double xPos, double yPos);
		void MouseButtonCallback(int button, int action, int mods);
		void KeyboardButtonCallback(int key, int scancode, int action, int mods);
		void MouseScrollCallback(double xOffset, double yOffset);
		void WindowReiszeCallback(int width, int height);

	private:
		GLFWwindow* mWindow{ nullptr };
		std::function<void(const WindowMouseMoveEventContext&)> mMouseMoveEventCallback = nullptr;
		std::function<void(const WindowMouseButtonEventContext&)> mMouseButtonCallback = nullptr;
		std::function<void(const WindowKeyboardKeyEventContext&)> mWindowKeyboardKeyCallback = nullptr;
		std::function<void(const WindowMouseScrollEventContext&)> mMouseScrollCallback = nullptr;
		std::function<void(const WindowResizeEventContext&)> mWindowResizeEventContext = nullptr;

	private:
		static WindowMouseButton _ToWindowMouseButton(int button);
		static WindowMouseButtonState _ToWindowMouseButtonState(int action);
		static WindowKeyboardKey _ToWindowKeyboardKey(int key);
		static WindowKeyboardKeyState _ToWindowKeyboardState(int action);

		static void _SetCursorPosCallback(GLFWwindow* window, double xPos, double yPos);
		static void _MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void _KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void _MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	};
}