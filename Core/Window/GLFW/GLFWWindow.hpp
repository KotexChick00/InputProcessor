#pragma once
#include <pch.h>
#include <Window/IWindow.hpp>
#include <Input/GLFW/GLFWKeyBoardInput.hpp>
#include <Input/GLFW/GLFWMouseInput.hpp>
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

		CORE_FORCE_INLINE std::function<void(const WindowMouseMoveEventContext&)> GetMouseMoveEventCallback() const { return mMouseMoveEventCallback; }
		CORE_FORCE_INLINE std::function<void(const WindowMouseButtonEventContext&)> GetMouseButtonEventCallback() const { return mMouseButtonCallback; }
		CORE_FORCE_INLINE std::function<void(const WindowKeyboardKeyEventContext&)> GetKeyboardKeyEventCallback() const { return mWindowKeyboardKeyCallback; }
		CORE_FORCE_INLINE std::function<void(const WindowMouseScrollEventContext&)> GetMouseScrollEventCallback() const { return mMouseScrollCallback; }
		CORE_FORCE_INLINE std::function<void(const WindowResizeEventContext&)> GetWindowResizeEventCallback() const { return mWindowResizeEventContext; }

		Input::InputState GetInput() const override;
		float GetCurrentSeconds() override;

		void Accept(IWindowVisitor* visitor) override;

		void EndFrame() override;

		GLFWwindow* GetNativeWindow();

	private:
		GLFWwindow* mWindow{ nullptr };
		std::function<void(const WindowMouseMoveEventContext&)> mMouseMoveEventCallback = nullptr;
		std::function<void(const WindowMouseButtonEventContext&)> mMouseButtonCallback = nullptr;
		std::function<void(const WindowKeyboardKeyEventContext&)> mWindowKeyboardKeyCallback = nullptr;
		std::function<void(const WindowMouseScrollEventContext&)> mMouseScrollCallback = nullptr;
		std::function<void(const WindowResizeEventContext&)> mWindowResizeEventContext = nullptr;

		Unique<Input::GLFW::GLFWKeyBoardInput> mKeyboardInput = nullptr;
		Unique<Input::GLFW::GLFWMouseInput> mMouseInput = nullptr;

	private:
		static WindowMouseButton _ToWindowMouseButton(int button);
		static WindowMouseButtonState _ToWindowMouseButtonState(int action);
		static WindowKeyboardKey _ToWindowKeyboardKey(int key);
		static WindowKeyboardKeyState _ToWindowKeyboardState(int action);
	};
}