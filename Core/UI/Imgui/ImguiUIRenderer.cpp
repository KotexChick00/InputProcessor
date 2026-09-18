#include <UI/Imgui/ImguiUIRenderer.hpp>

namespace InputProcessor::UI::Imgui {
	ImguiUIRenderer::ImguiUIRenderer(
		ImguiWindowContextInitVisitor* contextVisitor,
		ImguiWindowRenderVisitor* renderVisitor,
		ImguiWindowShutdownVisitor* shutdownVisitor
	) : mContextVisitor(contextVisitor), mRenderVisitor(renderVisitor), mShutdownVisitor(shutdownVisitor) { }

	void ImguiUIRenderer::Init(InputProcessor::Window::IWindow* window) {
		mWindow = window;
		mContextVisitor->Visit(mWindow);
	}

	void ImguiUIRenderer::Render() {
		mRenderVisitor->Visit(mWindow);
	}

	void ImguiUIRenderer::Free() {
		mShutdownVisitor->Visit(mWindow);
	}
}