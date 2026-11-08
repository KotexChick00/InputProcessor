#include <UI/Imgui/ImguiUIWindowVisitor.hpp>

using namespace CoreEngine::UI;
using namespace CoreEngine::Window;
using namespace CoreEngine::UI::Imgui;

// ImGui GLFW backend (imgui_impl_glfw) was removed when GLFW support was
// dropped from the engine. The visitor hierarchy is kept for forward
// compatibility — VisitWxWindow is a no-op so a wx-only build does not
// fail to link. If ImGui-wx integration is added later, implement
// VisitWxWindow in this file.

void ImguiWindowContextInitVisitor::Visit(IWindow* window) {
        window->Accept(this);
}

void ImguiWindowRenderVisitor::Visit(IWindow* window) {
        window->Accept(this);
}

void ImguiWindowShutdownVisitor::Visit(IWindow* window) {
        window->Accept(this);
}
