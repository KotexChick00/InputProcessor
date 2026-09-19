#pragma once
#include <Window/IWindow.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace CoreEngine::Window::GLFW {
	class GLFWWindow : public CoreEngine::Window::IWindow {
	public:
		void Init(const WindowConfiguration& config) override;
		void PollEvents() override;
		void SwapBuffers() override;
		bool CheckShouldClose() override;
		void Close() override;

		void Accept(IWindowVisitor* visitor) override;

		GLFWwindow* GetNativeWindow();

	private:
		GLFWwindow* mWindow{ nullptr };
	};
}