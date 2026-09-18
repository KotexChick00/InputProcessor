#pragma once
#include <iostream>
#include <Window/IWindowVisitor.hpp>

namespace InputProcessor::UI::Imgui {
	class ImguiWindowContextInitVisitor : public InputProcessor::Window::IWindowVisitor {
	public:
		void Visit(InputProcessor::Window::IWindow* window) override;
		void VisitGlfwWindow(InputProcessor::Window::GLFW::GLFWWindow* window) override;
	};

	class ImguiWindowRenderVisitor : public InputProcessor::Window::IWindowVisitor {
	public:
		void Visit(InputProcessor::Window::IWindow* window) override;
		void VisitGlfwWindow(InputProcessor::Window::GLFW::GLFWWindow* window) override;
	};

	class ImguiWindowShutdownVisitor : public InputProcessor::Window::IWindowVisitor {
	public:
		void Visit(InputProcessor::Window::IWindow* window) override;
		void VisitGlfwWindow(InputProcessor::Window::GLFW::GLFWWindow* window) override;
	};
}