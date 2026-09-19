#pragma once
#include <UI/IUIRenderer.hpp>
#include <UI/Imgui/ImguiUIWindowVisitor.hpp>

namespace CoreEngine::UI::Imgui {
	class ImguiUIRenderer : public IUIRenderer {
	public:
		ImguiUIRenderer(CoreEngine::Window::IWindow* window);

		void Init() override;
		void Render() override;
		void Free() override;

		~ImguiUIRenderer();

	private:
		ImguiWindowContextInitVisitor* mContextVisitor = nullptr;
		ImguiWindowRenderVisitor* mRenderVisitor = nullptr;
		ImguiWindowShutdownVisitor* mShutdownVisitor = nullptr;

		CoreEngine::Window::IWindow* mWindow = nullptr;
	};
}