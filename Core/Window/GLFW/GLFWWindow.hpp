#pragma once
#include <pch.h>
#include <Window/IWindow.hpp>
#include <Input/GLFW/GLFWKeyBoardInput.hpp>
#include <Input/GLFW/GLFWMouseInput.hpp>
#ifdef _USE_GLAD_OPENGL
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace CoreEngine
{
    class GLFWWindow : public IWindow
    {
    public:
        void Init(const WindowConfiguration& config) override;
        void PollEvents() override;
        void SwapBuffers() override;
        bool CheckShouldClose() override;
        void Close() override;

        void OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) override;
        void OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) override;
        void OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) override;
        void OnMouseSrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) override;
        void OnWindowReiszeEventCallback(std::function<void(const WindowSizeEventContext&)> callback) override;

        bool ShowOpenFileDialog(const WindowFileDialogOptions& options, std::string& outPath) override;
        void OnOpenFileRequestCallback(std::function<void()> callback) override;

        CORE_FORCE_INLINE std::function<void(const WindowMouseMoveEventContext&)> GetMouseMoveEventCallback() const { return mMouseMoveEventCallback; }
        CORE_FORCE_INLINE std::function<void(const WindowMouseButtonEventContext&)> GetMouseButtonEventCallback() const { return mMouseButtonCallback; }
        CORE_FORCE_INLINE std::function<void(const WindowKeyboardKeyEventContext&)> GetKeyboardKeyEventCallback() const { return mWindowKeyboardKeyCallback; }
        CORE_FORCE_INLINE std::function<void(const WindowMouseScrollEventContext&)> GetMouseScrollEventCallback() const { return mMouseScrollCallback; }
        CORE_FORCE_INLINE std::function<void(const WindowSizeEventContext&)> GetWindowResizeEventCallback() const { return mWindowResizeEventContext; }

        InputState GetInput() const override;
        float GetCurrentSeconds() override;

        void Accept(IWindowVisitor* visitor) override;

        void EndFrame() override;

        GLFWwindow* GetNativeWindow();

    private:
        GLFWwindow* mWindow{nullptr};
        std::function<void(const WindowMouseMoveEventContext&)> mMouseMoveEventCallback = nullptr;
        std::function<void(const WindowMouseButtonEventContext&)> mMouseButtonCallback = nullptr;
        std::function<void(const WindowKeyboardKeyEventContext&)> mWindowKeyboardKeyCallback = nullptr;
        std::function<void(const WindowMouseScrollEventContext&)> mMouseScrollCallback = nullptr;
        std::function<void(const WindowSizeEventContext&)> mWindowResizeEventContext = nullptr;
        std::function<void()> mOpenFileRequestCallback = nullptr;

        Unique<GLFWKeyBoardInput> mKeyboardInput = nullptr;
        Unique<GLFWMouseInput> mMouseInput = nullptr;

        // Per-window cursor delta tracker. Replaces the previous static
        // locals inside the GLFW cursor-pos callback, which leaked state
        // across windows and across create/destroy cycles.
        bool mHaveLastCursorPos = false;
        double mLastCursorX = 0.0;
        double mLastCursorY = 0.0;

    private:
        static WindowMouseButton ToWindowMouseButton(int button);
        static WindowMouseButtonState ToWindowMouseButtonState(int action);
        static WindowKeyboardKey ToWindowKeyboardKey(int key);
        static WindowKeyboardKeyState ToWindowKeyboardState(int action);
    };
}