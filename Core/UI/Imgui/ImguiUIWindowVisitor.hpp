#pragma once
#include <iostream>
#include <Window/IWindowVisitor.hpp>

namespace CoreEngine::UI::Imgui {
	class ImguiWindowContextInitVisitor : public CoreEngine::Window::IWindowVisitor {
	public:
		void Visit(CoreEngine::Window::IWindow* window) override;
		void VisitGlfwWindow(CoreEngine::Window::GLFW::GLFWWindow* window) override;
	};

	class ImguiWindowRenderVisitor : public CoreEngine::Window::IWindowVisitor {
	public:
		void Visit(CoreEngine::Window::IWindow* window) override;
		void VisitGlfwWindow(CoreEngine::Window::GLFW::GLFWWindow* window) override;
	};

	class ImguiWindowShutdownVisitor : public CoreEngine::Window::IWindowVisitor {
	public:
		void Visit(CoreEngine::Window::IWindow* window) override;
		void VisitGlfwWindow(CoreEngine::Window::GLFW::GLFWWindow* window) override;
	};
}