#include <UI/Imgui/ImguiUIRenderer.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::UI::Imgui {

        ImguiUIRenderer::ImguiUIRenderer(CoreEngine::Window::IWindow* window) : mWindow(window) { }

        // Destructor must be defined in the cpp (header only forward-declares the
        // visitor types via the included header). unique_ptr members handle cleanup.
        ImguiUIRenderer::~ImguiUIRenderer() {
                // If Free() was not explicitly called, still release visitors via
                // unique_ptr destructors. The ImGui context (if any) is also owned
                // via the shutdown visitor; if it was never created, this is a no-op.
                if (mShutdownVisitor && mContextVisitor && mWindow) {
                        mShutdownVisitor->Visit(mWindow);
                }
        }

        void ImguiUIRenderer::Init() {
                // Guard against double Init() which would leak the previous visitors
                // and create a second ImGui context.
                if (mContextVisitor) {
                        IP_ENGINE_WARN("ImguiUIRenderer::Init called twice; ignoring.");
                        return;
                }
                mContextVisitor  = std::make_unique<ImguiWindowContextInitVisitor>();
                mRenderVisitor   = std::make_unique<ImguiWindowRenderVisitor>();
                mShutdownVisitor = std::make_unique<ImguiWindowShutdownVisitor>();
                mContextVisitor->Visit(mWindow);
        }

        void ImguiUIRenderer::Render() {
                if (!mRenderVisitor) {
                        IP_ENGINE_WARN("ImguiUIRenderer::Render called before Init.");
                        return;
                }
                mRenderVisitor->Visit(mWindow);
        }

        void ImguiUIRenderer::Free() {
                if (mShutdownVisitor && mWindow) mShutdownVisitor->Visit(mWindow);
                mContextVisitor.reset();
                mRenderVisitor.reset();
                mShutdownVisitor.reset();
        }

} // namespace CoreEngine::UI::Imgui
