#include <UI/Imgui/ImguiUIWindowVisitor.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace CoreEngine::UI;
using namespace CoreEngine::Window;
using namespace CoreEngine::Window::GLFW;
using namespace CoreEngine::UI::Imgui;

void ImguiWindowContextInitVisitor::Visit(IWindow* window) {
	window->Accept(this);
}

void ImguiWindowContextInitVisitor::VisitGlfwWindow(GLFWWindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsDark();


	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window->GetNativeWindow(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init("#version 460");
}

void ImguiWindowRenderVisitor::Visit(IWindow* window) {
	window->Accept(this);
}

void ImguiWindowRenderVisitor::VisitGlfwWindow(GLFWWindow* window) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Simple Window");
	ImGui::Text("Hello from ImGui integrated into the app");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiWindowShutdownVisitor::Visit(IWindow* window) {
	window->Accept(this);
}

void ImguiWindowShutdownVisitor::VisitGlfwWindow(GLFWWindow* window) {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}