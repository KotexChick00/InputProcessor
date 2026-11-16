// CoreEngine demo: a spinning, interactive triangle that exercises every Core component.
//
//   Application  - subclass + OnInit/OnLoop/OnShutdown lifecycle, RequestClose()
//   Window       - default wxWidgets window created by WindowPlatformFactory
//   Renderer     - renderer configuration, shader + vertex buffer from the resource manager, draw calls
//   Event        - listeners for resize / keyboard / mouse button / scroll events
//   Input        - per-frame polling of keyboard and mouse state (pressed, just pressed, deltas)
//   Time         - delta time for frame-rate independent motion, FPS counter
//   Logger       - IP_CLIENT_* log macros
//   Memory       - LinearAllocator used as a per-frame scratch allocator
//
// Frame pacing is done by Application (ApplicationConfiguration::MaxFPS, 60 by default).

#include <Application/Application.hpp>
#include <Memory/LinearAllocator.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace
{
    constexpr auto VertexSource = R"(#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;
uniform vec2 uOffset;
uniform float uAngle;
uniform float uScale;
uniform float uAspect;
out vec3 vColor;
void main()
{
    mat2 rotation = mat2(cos(uAngle), sin(uAngle), -sin(uAngle), cos(uAngle));
    vec2 position = rotation * aPosition * uScale;
    position.x /= uAspect;
    gl_Position = vec4(position + uOffset, 0.0, 1.0);
    vColor = aColor;
})";

    constexpr auto FragmentSource = R"(#version 330 core
in vec3 vColor;
uniform vec4 uTint;
out vec4 oColor;
void main()
{
    oColor = vec4(vColor, 1.0) * uTint;
})";

    const CoreEngine::ColorRGBA ClearColors[] = {
        {0.08f, 0.09f, 0.12f, 1.0f},
        {0.20f, 0.10f, 0.15f, 1.0f},
        {0.05f, 0.18f, 0.12f, 1.0f},
        {0.90f, 0.90f, 0.92f, 1.0f},
    };

    struct FrameUniforms
    {
        float Tint[4];
    };

    class CoreDemo : public CoreEngine::Application
    {
    public:
        explicit CoreDemo(CoreEngine::ApplicationConfiguration& config)
            : Application(config)
            , mFrameAllocator(4 * 1024)
        {
        }

        void OnInitClient() override
        {
            PrintControls();
            CreateGpuResources();
            ApplyClearColor();
            SubscribeEvents();
            IP_CLIENT_DEBUG("Core demo initialized");
        }

        void OnLoopClient() override
        {
            const float deltaTime = GetTime()->GetDeltaTime();

            mFrameAllocator.Clear();
            HandleInput(deltaTime);
            if (!mPaused) mAngle += RotationSpeed * deltaTime;

            UpdateFpsCounter(deltaTime);
            Render();
        }

        void OnShutdownClient() override
        {
            // Both resources are owned by the renderer's resource manager.
            mShader = nullptr;
            mTriangle = nullptr;
            IP_CLIENT_DEBUG("Core demo shut down after {:.1f}s", mElapsed);
        }

    private:
        static constexpr float RotationSpeed = 1.5f; // radians per second
        static constexpr float MoveSpeed = 1.0f;     // NDC units per second
        static constexpr float MinScale = 0.1f;
        static constexpr float MaxScale = 2.0f;

        static void PrintControls()
        {
            std::cout << "CoreEngine demo controls\n"
                      << "  Arrows / WASD     move the triangle\n"
                      << "  Left mouse drag   move the triangle\n"
                      << "  Mouse wheel       scale\n"
                      << "  Space             pause / resume rotation\n"
                      << "  C                 cycle background color\n"
                      << "  R                 reset\n"
                      << "  Escape            quit\n";
        }

        void CreateGpuResources()
        {
            CoreEngine::IResourceManager* resources = GetRenderer()->GetResourceManager();

            mShader = resources->CreateShaderFromSources(VertexSource, FragmentSource);
            if (!mShader || !mShader->GetShaderId()) throw std::runtime_error("Failed to create demo shader");

            float positions[] = {
                0.0f, 0.6f,
                -0.52f, -0.3f,
                0.52f, -0.3f};
            float colors[] = {
                1.0f, 0.3f, 0.3f,
                0.3f, 1.0f, 0.3f,
                0.3f, 0.4f, 1.0f};

            mTriangle = resources->CreateVertexBuffer();
            if (!mTriangle) throw std::runtime_error("Failed to create demo vertex buffer");
            mTriangle->SetData(0, positions, sizeof(positions), 2, 2 * sizeof(float));
            mTriangle->SetData(1, colors, sizeof(colors), 3, 3 * sizeof(float));
        }

        void SubscribeEvents()
        {
            CoreEngine::EventDispatcher* dispatcher = GetEventDispatcher();

            dispatcher->AddEventListener<CoreEngine::WindowResizeEventContext>([](const CoreEngine::WindowResizeEventContext& e) {
                IP_CLIENT_DEBUG("Window resized to {}x{}", e.GetWidth(), e.GetHeight());
                return false;
            });

            dispatcher->AddEventListener<CoreEngine::KeyBoardKeyEventContext>([this](const CoreEngine::KeyBoardKeyEventContext& e) {
                if (e.GetState() != CoreEngine::KeyboardKeyState::Pressed) return false;

                switch (e.GetKey())
                {
                case CoreEngine::KeyboardKey::Space:
                    mPaused = !mPaused;
                    IP_CLIENT_DEBUG("Rotation {}", mPaused ? "paused" : "resumed");
                    return true;
                case CoreEngine::KeyboardKey::C:
                    mClearColorIndex = (mClearColorIndex + 1) % std::size(ClearColors);
                    ApplyClearColor();
                    return true;
                default:
                    return false;
                }
            });

            dispatcher->AddEventListener<CoreEngine::MouseButtonEventContext>([](const CoreEngine::MouseButtonEventContext& e) {
                const char* state = e.GetButtonState() == CoreEngine::MouseButtonState::Released ? "released" : "pressed";
                IP_CLIENT_TRACE("Mouse button {} {}", static_cast<int>(e.GetButton()), state);
                return false;
            });

            dispatcher->AddEventListener<CoreEngine::MouseScrollEventContext>([this](const CoreEngine::MouseScrollEventContext& e) {
                mScale = std::clamp(mScale * std::pow(1.1f, static_cast<float>(e.GetDeltaY())), MinScale, MaxScale);
                return false;
            });
        }

        void HandleInput(float deltaTime)
        {
            const CoreEngine::InputState& input = GetInput();
            CoreEngine::KeyboardInput* keyboard = input.KeyboardInput;
            CoreEngine::MouseInput* mouse = input.MouseInput;
            if (!keyboard || !mouse) return;

            if (keyboard->CheckIsPressed(CoreEngine::KeyboardKey::Escape)) RequestClose();
            if (keyboard->CheckIsJustPressed(CoreEngine::KeyboardKey::R)) Reset();

            float moveX = 0.0f, moveY = 0.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Left, CoreEngine::KeyboardKey::A)) moveX -= 1.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Right, CoreEngine::KeyboardKey::D)) moveX += 1.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Up, CoreEngine::KeyboardKey::W)) moveY += 1.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Down, CoreEngine::KeyboardKey::S)) moveY -= 1.0f;
            mOffsetX += moveX * MoveSpeed * deltaTime;
            mOffsetY += moveY * MoveSpeed * deltaTime;

            // Mouse deltas are in pixels; convert to normalized device coordinates (Y points up).
            if (mouse->CheckIsPressed(CoreEngine::MouseButton::ButtonLeft))
            {
                CoreEngine::ViewPortOptions viewport = GetRenderer()->GetConfig().ViewPortOptions;
                mOffsetX += mouse->GetDeltaX() * 2.0f / std::max(1u, viewport.Width);
                mOffsetY -= mouse->GetDeltaY() * 2.0f / std::max(1u, viewport.Height);
            }

            mOffsetX = std::clamp(mOffsetX, -1.0f, 1.0f);
            mOffsetY = std::clamp(mOffsetY, -1.0f, 1.0f);
        }

        void Render()
        {
            CoreEngine::IRenderer* renderer = GetRenderer();
            CoreEngine::ViewPortOptions viewport = renderer->GetConfig().ViewPortOptions;
            float aspect = static_cast<float>(viewport.Width) / std::max(1u, viewport.Height);

            // Per-frame data lives in the scratch allocator; it is reclaimed by Clear() next frame.
            auto* uniforms = static_cast<FrameUniforms*>(mFrameAllocator.Allocate(sizeof(FrameUniforms), alignof(FrameUniforms)));
            float pulse = mPaused ? 1.0f : 0.85f + 0.15f * std::sin(mElapsed * 3.0f);
            *uniforms = {{pulse, pulse, pulse, 1.0f}};

            renderer->GetRendererCommand()->ClearBuffers(CoreEngine::ClearBufferMasks::Color);

            mShader->Use();
            mShader->SetUniform2f("uOffset", mOffsetX, mOffsetY);
            mShader->SetUniform1f("uAngle", mAngle);
            mShader->SetUniform1f("uScale", mScale);
            mShader->SetUniform1f("uAspect", aspect);
            mShader->SetUniform4f("uTint", uniforms->Tint[0], uniforms->Tint[1], uniforms->Tint[2], uniforms->Tint[3]);

            mTriangle->Bind();
            renderer->GetRendererCommand()->Draw(CoreEngine::RenderMode::Triangles, 3);
            mTriangle->UnBind();
            mShader->Unuse();
        }

        void UpdateFpsCounter(float deltaTime)
        {
            mElapsed += deltaTime;
            mFpsTime += deltaTime;
            ++mFpsFrames;
            if (mFpsTime >= 2.0f)
            {
                IP_CLIENT_DEBUG("{:.1f} FPS (scale {:.2f}, offset {:.2f}, {:.2f})", mFpsFrames / mFpsTime, mScale, mOffsetX, mOffsetY);
                mFpsTime = 0.0f;
                mFpsFrames = 0;
            }
        }

        void ApplyClearColor()
        {
            CoreEngine::IRenderer* renderer = GetRenderer();
            CoreEngine::RendererConfiguration config = renderer->GetConfig();
            config.ClearBufferColor = ClearColors[mClearColorIndex];
            renderer->Config(config);
        }

        void Reset()
        {
            mOffsetX = 0.0f;
            mOffsetY = 0.0f;
            mAngle = 0.0f;
            mScale = 1.0f;
            mPaused = false;
            IP_CLIENT_DEBUG("Demo state reset");
        }

        static bool IsAnyPressed(CoreEngine::KeyboardInput* keyboard, CoreEngine::KeyboardKey first, CoreEngine::KeyboardKey second)
        {
            return keyboard->CheckIsPressed(first) || keyboard->CheckIsPressed(second);
        }

    private:
        CoreEngine::LinearAllocator mFrameAllocator;

        CoreEngine::IShader* mShader = nullptr;
        CoreEngine::IVertexBuffer* mTriangle = nullptr;

        float mOffsetX = 0.0f;
        float mOffsetY = 0.0f;
        float mAngle = 0.0f;
        float mScale = 1.0f;
        bool mPaused = false;
        size_t mClearColorIndex = 0;

        float mElapsed = 0.0f;
        float mFpsTime = 0.0f;
        int mFpsFrames = 0;
    };
}

int main()
{
    try
    {
        CoreEngine::ApplicationConfiguration config{960, 640, "CoreEngine Demo"};
        CoreDemo demo(config);
        demo.Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}
