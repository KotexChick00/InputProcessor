#pragma once
#include <Window/IWindow.hpp>
#include <Window/GLFW/GLFWWindow.hpp>

namespace CoreEngine::Window {

	class IWindowVisitor {
	public:
		virtual void Visit(IWindow* window) = 0;
		virtual void VisitGlfwWindow(GLFW::GLFWWindow* window) = 0;
	};
}