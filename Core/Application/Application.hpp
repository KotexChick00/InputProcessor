#pragma once
#include <Event/EventContext.hpp>
#include <Window/IWindow.hpp>
#include <Window/WindowPlatformFactory.hpp>
#include <Renderer/IRenderer.hpp>
#include <Input/InputState.hpp>
#include <Time/Time.hpp>
#include <Event/EventDispatcher.hpp>
#include <pch.h>

namespace CoreEngine
{
    enum class CORE_API RenderAPI
    {
        Opengl,
        DirectX,
        Vulkan,
        Injection // This will tell the application that the configuration will use the Injected RenderAPI
    };

    struct CORE_API ApplicationConfiguration
    {
        unsigned int Width;
        unsigned int Height;
        const char* Title;

        unsigned int MaxFPS = 60;

        CoreEngine::WindowPlatformSpec WindowPlatformSpec = WindowPlatformSpec::Wx;
        IWindow* InjectedWindow = nullptr; // If WindowPlatformSpec is Injection

        CoreEngine::RenderAPI RenderAPI = RenderAPI::Opengl;
        IRenderer* InjectedRenderer = nullptr; // If RenderAPI is Injection
    };

    class CORE_API Application
    {
    public:
        virtual ~Application() = default;

        void Run();
        void RequestClose() { mCloseRequested = true; }

        // Client
        virtual void OnInitClient() { }
        virtual void OnLoopClient() { }
        virtual void OnShutdownClient() { }

        // Client API
        CORE_FORCE_INLINE IRenderer* GetRenderer() const { return mRenderer.get(); }
        CORE_FORCE_INLINE Time* GetTime() const { return Time::GetInstance(); }
        CORE_FORCE_INLINE const InputState& GetInput() const { return mInputState; }
        CORE_FORCE_INLINE EventDispatcher* GetEventDispatcher() const { return (EventDispatcher*)&mEventDispatcher; }

    protected:
        Application(ApplicationConfiguration& configuration);

    private:
        bool Init();
        void BeginLoop();
        void EndLoop();
        void Shutdown();

        void SetupLogger();
        void SetupWindow();
        void SetupEvents();
        void SetupRenderer();

    private:
        ApplicationConfiguration mConfig;
        bool mCloseRequested = false;
        Unique<IWindow> mWindow = nullptr;
        Unique<IRenderer> mRenderer = nullptr;

        EventDispatcher mEventDispatcher;
        InputState mInputState;

    private:
        static KeyboardKey ToApplicationKeyboardKey(WindowKeyboardKey key);
        static KeyboardKeyState ToApplicationKeyboardKeyState(WindowKeyboardKeyState keyState);
        static MouseButton ToApplicationMouseButton(WindowMouseButton mouseButton);
        static MouseButtonState ToApplicationMouseButtonState(WindowMouseButtonState mouseButtonState);
    };
}
