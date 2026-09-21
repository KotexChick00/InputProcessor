#include <UI/Imgui/ImguiUIWindowVisitor.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Window/SDL3/SDL3Window.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

using namespace CoreEngine::UI;
using namespace CoreEngine::Window;
using namespace CoreEngine::Window::GLFW;
using namespace CoreEngine::Window::SDL3;
using namespace CoreEngine::UI::Imgui;

namespace {
	void CreateImguiContext() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		ImGui::StyleColorsDark();
	}

	// Phần UI dùng chung cho mọi backend
	void DrawUI() {
		ImGui::NewFrame();
		ImGui::Begin("Simple Window");
		ImGui::Text("Hello from ImGui integrated into the app");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
			1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

// ---------- Init ----------
void ImguiWindowContextInitVisitor::Visit(IWindow* window) { window->Accept(this); }

void ImguiWindowContextInitVisitor::VisitGlfwWindow(GLFWWindow* window) {
	CreateImguiContext();
	ImGui_ImplGlfw_InitForOpenGL(window->GetNativeWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void ImguiWindowContextInitVisitor::VisitSdl3Window(SDL3Window* window) {
	CreateImguiContext();
	ImGui_ImplSDL3_InitForOpenGL(window->GetNativeWindow(), window->GetGLContext());
	ImGui_ImplOpenGL3_Init("#version 460");

	// SDL không tự cài callback như GLFW: phải chuyển từng event cho ImGui
	window->AddNativeEventHook([](const SDL_Event& e) {
		ImGui_ImplSDL3_ProcessEvent(&e);
		});
}

// ---------- Render ----------
void ImguiWindowRenderVisitor::Visit(IWindow* window) { window->Accept(this); }

void ImguiWindowRenderVisitor::VisitGlfwWindow(GLFWWindow*) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	DrawUI();
}

void ImguiWindowRenderVisitor::VisitSdl3Window(SDL3Window*) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	DrawUI();
}

// ---------- Shutdown ----------
void ImguiWindowShutdownVisitor::Visit(IWindow* window) { window->Accept(this); }

void ImguiWindowShutdownVisitor::VisitGlfwWindow(GLFWWindow*) {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImguiWindowShutdownVisitor::VisitSdl3Window(SDL3Window*) {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}