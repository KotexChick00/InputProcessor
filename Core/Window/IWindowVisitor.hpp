#pragma once
#include <Window/IWindow.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Window/SDL3/SDL3Window.hpp>

namespace CoreEngine::Window {
	class CORE_API IWindowVisitor {
	public:
		virtual ~IWindowVisitor() = default;
		virtual void Visit(IWindow* window) = 0;
		virtual void VisitGlfwWindow(GLFW::GLFWWindow* window) = 0;
		virtual void VisitSdl3Window(SDL3::SDL3Window* window) = 0;
	};
}