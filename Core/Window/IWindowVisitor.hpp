#pragma once
#include <Window/IWindow.hpp>
#include <Window/GLFW/GLFWWindow.hpp>

// Forward-declare WxWindow to avoid pulling in all wxWidgets headers
// into every visitor (they are only needed in .cpp files that use WxWindow).
namespace CoreEngine::Window::Wx { class WxWindow; }

namespace CoreEngine::Window {
	class CORE_API IWindowVisitor {
	public:
		virtual void Visit(IWindow* window) = 0;
		virtual void VisitGlfwWindow(GLFW::GLFWWindow* window) = 0;
		virtual void VisitWxWindow(Wx::WxWindow* window) = 0;
	};
}