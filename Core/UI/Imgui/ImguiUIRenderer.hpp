#pragma once
#include <UI/IUIRenderer.hpp>
#include <UI/Imgui/ImguiUIWindowVisitor.hpp>
#include <memory>

namespace CoreEngine::UI::Imgui {
        class ImguiUIRenderer : public IUIRenderer {
        public:
                explicit ImguiUIRenderer(CoreEngine::Window::IWindow* window);
                ~ImguiUIRenderer() override;

                void Init() override;
                void Render() override;
                void Free() override;

        private:
                // Owning pointers; raw `new`/`delete` would leak if Init() is
                // forgotten or double-called, so we use unique_ptr throughout.
                std::unique_ptr<ImguiWindowContextInitVisitor> mContextVisitor;
                std::unique_ptr<ImguiWindowRenderVisitor>     mRenderVisitor;
                std::unique_ptr<ImguiWindowShutdownVisitor>   mShutdownVisitor;

                // Non-owning: borrowed from the application.
                CoreEngine::Window::IWindow* mWindow = nullptr;
        };
}