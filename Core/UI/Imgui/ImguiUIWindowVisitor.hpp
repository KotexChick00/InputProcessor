#pragma once
#include <iostream>
#include <Window/IWindowVisitor.hpp>

namespace CoreEngine::UI::Imgui {

	class ImguiWindowContextInitVisitor : public CoreEngine::Window::IWindowVisitor {
	public:
		void Visit(CoreEngine::Window::IWindow* window) override;
		void VisitGlfwWindow(CoreEngine::Window::GLFW::GLFWWindow* window) override;
		// ImGui không hỗ trợ wxWidgets context — no-op để tránh lỗi compile
		void VisitWxWindow(CoreEngine::Window::Wx::WxWindow* window) override {}
	};

	class ImguiWindowRenderVisitor : public CoreEngine::Window::IWindowVisitor {
	public:
		void Visit(CoreEngine::Window::IWindow* window) override;
		void VisitGlfwWindow(CoreEngine::Window::GLFW::GLFWWindow* window) override;
		// ImGui không hỗ trợ wxWidgets render — no-op
		void VisitWxWindow(CoreEngine::Window::Wx::WxWindow* window) override {}
	};

	class ImguiWindowShutdownVisitor : public CoreEngine::Window::IWindowVisitor {
	public:
		void Visit(CoreEngine::Window::IWindow* window) override;
		void VisitGlfwWindow(CoreEngine::Window::GLFW::GLFWWindow* window) override;
		// ImGui không hỗ trợ wxWidgets shutdown — no-op
		void VisitWxWindow(CoreEngine::Window::Wx::WxWindow* window) override {}
	};

}