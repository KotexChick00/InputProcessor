#include <UI/Imgui/ImguiUIRenderer.hpp>

namespace InputProcessor::UI::Imgui {
	ImguiUIRenderer::ImguiUIRenderer(InputProcessor::Window::IWindow* window) : mWindow(window) { }

	void ImguiUIRenderer::Init() {
		mContextVisitor = new ImguiWindowContextInitVisitor();
		mRenderVisitor = new ImguiWindowRenderVisitor();
		mShutdownVisitor = new ImguiWindowShutdownVisitor();
		mContextVisitor->Visit(mWindow);
	}

	void ImguiUIRenderer::Render() {
		mRenderVisitor->Visit(mWindow);
	}

	void ImguiUIRenderer::Free() {
		mShutdownVisitor->Visit(mWindow);
		delete mContextVisitor;
		delete mRenderVisitor;
		delete mShutdownVisitor;
	}
}