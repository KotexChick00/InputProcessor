#pragma once
#include <UI/IUIRenderer.hpp>
#include <UI/Imgui/ImguiUIWindowVisitor.hpp>

namespace InputProcessor::UI::Imgui {
	class ImguiUIRenderer : public IUIRenderer {
	public:
		ImguiUIRenderer(ImguiWindowContextInitVisitor* contextVisitor, ImguiWindowRenderVisitor* renderVisitor, ImguiWindowShutdownVisitor* shutdownVisitor);

		void Init(InputProcessor::Window::IWindow* window) override;
		void Render() override;
		void Free() override;
	private:
		ImguiWindowContextInitVisitor* mContextVisitor = nullptr;
		ImguiWindowRenderVisitor* mRenderVisitor = nullptr;
		ImguiWindowShutdownVisitor* mShutdownVisitor = nullptr;

		InputProcessor::Window::IWindow* mWindow = nullptr;
	};
}