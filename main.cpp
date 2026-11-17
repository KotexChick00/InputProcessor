// CoreEngine demo: a swarm of triangles driven by the Entity Component System (Core/ECS)
// and the memory allocators (Core/Memory).
//
//   ECS::Actor        - every triangle is an Actor created with ECS::CreateActor(); the player is one too
//   Components        - Transform, Velocity, Spin, Tint, Lifetime, PlayerControl (plain data, IComponent)
//   ComponentManager  - components live in MemoryChunkManager blocks, systems walk them with
//                       GetAllComponents<T>() and look up siblings with GetComponentByEntityID<T>()
//   SystemManager     - systems run in a topological order (Kahn) built from AddDependency:
//                       PlayerInput -> Movement -> Lifetime -> Render, Spin -> Render
//   EntityManager     - expired actors are destroyed and their ids are recycled by the next spawn
//   MemoryManager     - per-frame allocator: list of expired actors, reset every frame by Update()
//                       stack allocator: spawn palette allocated at init, freed at shutdown
//
// Frame pacing is done by Application (ApplicationConfiguration::MaxFPS, 60 by default).

#include <Application/Application.hpp>
#include <ECS/ECS.hpp>
#include <Memory/MemoryManager.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <stdexcept>

namespace ECS = CoreEngine::ECS;

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

    constexpr size_t SpawnBurstCount = 25;
    constexpr size_t MaxActorCount = 2000;
    constexpr size_t PaletteSize = 6;

    // -------------------------------------------------------------------------
    // Components: plain data, no behaviour
    // -------------------------------------------------------------------------

    struct Transform : public ECS::IComponent
    {
        Transform(float x, float y, float scale)
            : X(x)
            , Y(y)
            , Scale(scale)
        {
        }
        float X = 0.0f;
        float Y = 0.0f;
        float Angle = 0.0f;
        float Scale = 1.0f;
    };

    struct Velocity : public ECS::IComponent
    {
        Velocity(float x = 0.0f, float y = 0.0f)
            : X(x)
            , Y(y)
        {
        }
        float X;
        float Y;
    };

    struct Spin : public ECS::IComponent
    {
        explicit Spin(float speed)
            : Speed(speed)
        {
        }
        float Speed; // radians per second
    };

    struct Tint : public ECS::IComponent
    {
        Tint(float r, float g, float b)
            : R(r)
            , G(g)
            , B(b)
        {
        }
        float R;
        float G;
        float B;
    };

    struct Lifetime : public ECS::IComponent
    {
        explicit Lifetime(float duration)
            : Remaining(duration)
            , Duration(duration)
        {
        }
        float Remaining;
        float Duration;
    };

    // Tag component: marks the actor moved by the keyboard.
    struct PlayerControl : public ECS::IComponent
    {
        float Speed = 1.2f; // NDC units per second
    };

    struct Color
    {
        float R;
        float G;
        float B;
    };

    // -------------------------------------------------------------------------
    // Systems: behaviour, run by ECS::Update in dependency order
    // -------------------------------------------------------------------------

    class PlayerInputSystem : public ECS::ISystem
    {
    public:
        explicit PlayerInputSystem(const CoreEngine::InputState& input)
            : ISystem(ECS::ESystemPriority::High)
            , mInput(input)
        {
        }

        void OnUpdate(float) override
        {
            CoreEngine::KeyboardInput* keyboard = mInput.KeyboardInput;
            if (!keyboard)
            {
                return;
            }

            float moveX = 0.0f, moveY = 0.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Left, CoreEngine::KeyboardKey::A)) moveX -= 1.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Right, CoreEngine::KeyboardKey::D)) moveX += 1.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Up, CoreEngine::KeyboardKey::W)) moveY += 1.0f;
            if (IsAnyPressed(keyboard, CoreEngine::KeyboardKey::Down, CoreEngine::KeyboardKey::S)) moveY -= 1.0f;

            ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
            for (PlayerControl& player : components.GetAllComponents<PlayerControl>())
            {
                if (Velocity* velocity = components.GetComponentByEntityID<Velocity>(player.GetOwner()))
                {
                    velocity->X = moveX * player.Speed;
                    velocity->Y = moveY * player.Speed;
                }
            }
        }

    private:
        static bool IsAnyPressed(CoreEngine::KeyboardInput* keyboard, CoreEngine::KeyboardKey first, CoreEngine::KeyboardKey second)
        {
            return keyboard->CheckIsPressed(first) || keyboard->CheckIsPressed(second);
        }

    private:
        const CoreEngine::InputState& mInput; // Owned by the application
    };

    class MovementSystem : public ECS::ISystem
    {
    public:
        void OnUpdate(float deltaTime) override
        {
            ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
            for (Velocity& velocity : components.GetAllComponents<Velocity>())
            {
                Transform* transform = components.GetComponentByEntityID<Transform>(velocity.GetOwner());
                if (!transform)
                {
                    continue;
                }

                transform->X += velocity.X * deltaTime;
                transform->Y += velocity.Y * deltaTime;

                // The player stops at the border, the swarm bounces off it.
                if (components.HasComponent<PlayerControl>(velocity.GetOwner()))
                {
                    transform->X = std::clamp(transform->X, -1.0f, 1.0f);
                    transform->Y = std::clamp(transform->Y, -1.0f, 1.0f);
                    continue;
                }
                Bounce(transform->X, velocity.X);
                Bounce(transform->Y, velocity.Y);
            }
        }

    private:
        static void Bounce(float& position, float& speed)
        {
            if (position < -1.0f || position > 1.0f)
            {
                position = std::clamp(position, -1.0f, 1.0f);
                speed = -speed;
            }
        }
    };

    class SpinSystem : public ECS::ISystem
    {
    public:
        void OnUpdate(float deltaTime) override
        {
            ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
            for (Spin& spin : components.GetAllComponents<Spin>())
            {
                if (Transform* transform = components.GetComponentByEntityID<Transform>(spin.GetOwner()))
                {
                    transform->Angle += spin.Speed * deltaTime;
                }
            }
        }
    };

    // Destroys actors whose lifetime ran out. Destroying while iterating would invalidate the view,
    // so the expired ids are collected in per-frame memory first.
    class LifetimeSystem : public ECS::ISystem
    {
    public:
        explicit LifetimeSystem(CoreEngine::MemoryManager& memory)
            : mMemory(memory)
        {
        }

        void OnUpdate(float deltaTime) override
        {
            ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
            ECS::ComponentView<Lifetime> lifetimes = components.GetAllComponents<Lifetime>();
            if (lifetimes.Empty())
            {
                return;
            }

            void* memory = mMemory.AllocateOnPerframe(sizeof(ECS::EntityID) * lifetimes.Size(), alignof(ECS::EntityID));
            if (!memory)
            {
                return;
            }

            ECS::EntityID* expired = static_cast<ECS::EntityID*>(memory);
            size_t expiredCount = 0;
            for (Lifetime& lifetime : lifetimes)
            {
                lifetime.Remaining -= deltaTime;
                if (lifetime.Remaining <= 0.0f)
                {
                    expired[expiredCount++] = lifetime.GetOwner();
                }
            }

            for (size_t index = 0; index < expiredCount; ++index)
            {
                ECS::Actor actor = ECS::GetActor(expired[index]);
                ECS::DestroyActor(actor);
            }
            mDestroyedCount += expiredCount;
        }

        size_t GetDestroyedCount() const { return mDestroyedCount; }

    private:
        CoreEngine::MemoryManager& mMemory; // Owned by the application
        size_t mDestroyedCount = 0;
    };

    class RenderSystem : public ECS::ISystem
    {
    public:
        RenderSystem(CoreEngine::IRenderer* renderer, CoreEngine::IShader* shader, CoreEngine::IVertexBuffer* triangle)
            : ISystem(ECS::ESystemPriority::Low)
            , mRenderer(renderer)
            , mShader(shader)
            , mTriangle(triangle)
        {
        }

        void OnUpdate(float) override
        {
            CoreEngine::ViewPortOptions viewport = mRenderer->GetConfig().ViewPortOptions;
            float aspect = static_cast<float>(viewport.Width) / std::max(1u, viewport.Height);

            mRenderer->GetRendererCommand()->ClearBuffers(CoreEngine::ClearBufferMasks::Color);
            mShader->Use();
            mShader->SetUniform1f("uAspect", aspect);
            mTriangle->Bind();

            ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
            for (const Transform& transform : components.GetAllComponents<Transform>())
            {
                float r = 1.0f, g = 1.0f, b = 1.0f;
                if (const Tint* tint = components.GetComponentByEntityID<Tint>(transform.GetOwner()))
                {
                    r = tint->R;
                    g = tint->G;
                    b = tint->B;
                }
                // Fade out during the last second of life.
                float fade = 1.0f;
                if (const Lifetime* lifetime = components.GetComponentByEntityID<Lifetime>(transform.GetOwner()))
                {
                    fade = std::clamp(lifetime->Remaining, 0.0f, 1.0f);
                }

                mShader->SetUniform2f("uOffset", transform.X, transform.Y);
                mShader->SetUniform1f("uAngle", transform.Angle);
                mShader->SetUniform1f("uScale", transform.Scale);
                mShader->SetUniform4f("uTint", r * fade, g * fade, b * fade, 1.0f);
                mRenderer->GetRendererCommand()->Draw(CoreEngine::RenderMode::Triangles, 3);
            }

            mTriangle->UnBind();
            mShader->Unuse();
        }

    private:
        CoreEngine::IRenderer* mRenderer;       // Borrowed
        CoreEngine::IShader* mShader;           // Borrowed, owned by the resource manager
        CoreEngine::IVertexBuffer* mTriangle;   // Borrowed, owned by the resource manager
    };

    // -------------------------------------------------------------------------
    // Application
    // -------------------------------------------------------------------------

    class EcsDemo : public CoreEngine::Application
    {
    public:
        explicit EcsDemo(CoreEngine::ApplicationConfiguration& config)
            : Application(config)
            , mMemory(CreateMemoryConfiguration())
        {
        }

        void OnInitClient() override
        {
            PrintControls();
            CreateGpuResources();
            CreatePalette();
            CreateSystems();
            SubscribeEvents();
            Reset();
            IP_CLIENT_DEBUG("ECS demo initialized");
        }

        void OnLoopClient() override
        {
            const float deltaTime = GetTime()->GetDeltaTime();

            mMemory.Update(); // Frees last frame's per-frame allocations.
            HandleInput();
            ECS::Update(deltaTime);
            UpdateStats(deltaTime);
        }

        void OnShutdownClient() override
        {
            // Systems hold renderer resources: destroy the ECS while the renderer and logger are alive.
            ECS::Shutdown();
            mPlayer = ECS::Actor();

            if (mPalette)
            {
                mMemory.FreeOnStack(mPalette);
                mPalette = nullptr;
            }
            mMemory.DetectMemoryLeak();

            mShader = nullptr;
            mTriangle = nullptr;
            IP_CLIENT_DEBUG("ECS demo shut down after {:.1f}s", mElapsed);
        }

    private:
        static CoreEngine::MemoryConfiguration CreateMemoryConfiguration()
        {
            CoreEngine::MemoryConfiguration config;
            config.LinearAllocateSize = 64 * 1024;
            config.StackAllocateSize = 4 * 1024;
            return config;
        }

        static void PrintControls()
        {
            std::cout << "CoreEngine ECS demo controls\n"
                      << "  Arrows / WASD     move the player (white triangle)\n"
                      << "  Space             spawn a burst of " << SpawnBurstCount << " actors\n"
                      << "  Mouse wheel       scale the player\n"
                      << "  P                 pause / resume the simulation (disables systems)\n"
                      << "  Backspace         destroy every actor except the player\n"
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
                1.0f, 1.0f, 1.0f,
                0.8f, 0.8f, 0.8f,
                0.6f, 0.6f, 0.6f};

            mTriangle = resources->CreateVertexBuffer();
            if (!mTriangle) throw std::runtime_error("Failed to create demo vertex buffer");
            mTriangle->SetData(0, positions, sizeof(positions), 2, 2 * sizeof(float));
            mTriangle->SetData(1, colors, sizeof(colors), 3, 3 * sizeof(float));

            CoreEngine::RendererConfiguration config = GetRenderer()->GetConfig();
            config.ClearBufferColor = {0.08f, 0.09f, 0.12f, 1.0f};
            GetRenderer()->Config(config);
        }

        // The palette lives on the MemoryManager stack for the whole run and is freed at shutdown.
        void CreatePalette()
        {
            void* memory = mMemory.AllocateOnStack(sizeof(Color) * PaletteSize, alignof(Color));
            if (!memory) throw std::runtime_error("Failed to allocate the palette");

            static constexpr Color Colors[PaletteSize] = {
                {1.00f, 0.35f, 0.35f},
                {0.35f, 1.00f, 0.45f},
                {0.35f, 0.55f, 1.00f},
                {1.00f, 0.85f, 0.30f},
                {0.85f, 0.40f, 1.00f},
                {0.30f, 0.95f, 0.95f},
            };
            mPalette = static_cast<Color*>(memory);
            std::uninitialized_copy(std::begin(Colors), std::end(Colors), mPalette);
        }

        void CreateSystems()
        {
            ECS::SystemManager& systems = ECS::SystemManager::GetInstance();
            systems.AddSystem<PlayerInputSystem>(GetInput());
            systems.AddSystem<MovementSystem>();
            systems.AddSystem<SpinSystem>();
            mLifetimeSystem = systems.AddSystem<LifetimeSystem>(mMemory);
            systems.AddSystem<RenderSystem>(GetRenderer(), mShader, mTriangle);

            // Execution order: PlayerInput -> Movement -> Lifetime -> Render, Spin -> Render.
            systems.AddDependency<MovementSystem, PlayerInputSystem>();
            systems.AddDependency<LifetimeSystem, MovementSystem>();
            systems.AddDependency<RenderSystem, MovementSystem>();
            systems.AddDependency<RenderSystem, SpinSystem>();
            systems.AddDependency<RenderSystem, LifetimeSystem>();
        }

        void SubscribeEvents()
        {
            CoreEngine::EventDispatcher* dispatcher = GetEventDispatcher();

            dispatcher->AddEventListener<CoreEngine::KeyBoardKeyEventContext>([this](const CoreEngine::KeyBoardKeyEventContext& e)
            {
                if (e.GetState() != CoreEngine::KeyboardKeyState::Pressed) return false;

                switch (e.GetKey())
                {
                case CoreEngine::KeyboardKey::Space:
                    SpawnBurst();
                    return true;
                case CoreEngine::KeyboardKey::P:
                    TogglePause();
                    return true;
                case CoreEngine::KeyboardKey::Backspace:
                    DestroySwarm();
                    return true;
                default:
                    return false;
                }
            });

            dispatcher->AddEventListener<CoreEngine::MouseScrollEventContext>([this](const CoreEngine::MouseScrollEventContext& e)
            {
                if (Transform* transform = mPlayer.GetComponent<Transform>())
                {
                    transform->Scale = std::clamp(transform->Scale * std::pow(1.1f, static_cast<float>(e.GetDeltaY())), 0.05f, 0.8f);
                }
                return false;
            });
        }

        void HandleInput()
        {
            CoreEngine::KeyboardInput* keyboard = GetInput().KeyboardInput;
            if (!keyboard) return;

            if (keyboard->CheckIsPressed(CoreEngine::KeyboardKey::Escape)) RequestClose();
            if (keyboard->CheckIsJustPressed(CoreEngine::KeyboardKey::R)) Reset();
        }

        void Reset()
        {
            DestroySwarm();
            if (mPlayer.IsValid()) ECS::DestroyActor(mPlayer);

            mPlayer = ECS::CreateActor();
            mPlayer.AddComponent<Transform>(0.0f, 0.0f, 0.2f);
            mPlayer.AddComponent<Velocity>();
            mPlayer.AddComponent<Tint>(1.0f, 1.0f, 1.0f);
            mPlayer.AddComponent<PlayerControl>();

            if (mPaused) TogglePause();
            SpawnBurst();
            IP_CLIENT_DEBUG("Demo state reset, player is entity {}", mPlayer.GetID());
        }

        void SpawnBurst()
        {
            const Transform* origin = mPlayer.GetComponent<Transform>();
            std::uniform_real_distribution<float> angle(0.0f, 6.2831853f);
            std::uniform_real_distribution<float> speed(0.2f, 0.9f);
            std::uniform_real_distribution<float> spin(-4.0f, 4.0f);
            std::uniform_real_distribution<float> scale(0.03f, 0.09f);
            std::uniform_real_distribution<float> lifetime(3.0f, 8.0f);
            std::uniform_int_distribution<size_t> color(0, PaletteSize - 1);

            size_t spawned = 0;
            for (; spawned < SpawnBurstCount && ECS::EntityManager::GetInstance().GetAliveCount() < MaxActorCount; ++spawned)
            {
                float direction = angle(mRandom);
                float startSpeed = speed(mRandom);
                const Color& tint = mPalette[color(mRandom)];

                ECS::Actor actor = ECS::CreateActor();
                actor.AddComponent<Transform>(origin ? origin->X : 0.0f, origin ? origin->Y : 0.0f, scale(mRandom));
                actor.AddComponent<Velocity>(std::cos(direction) * startSpeed, std::sin(direction) * startSpeed);
                actor.AddComponent<Spin>(spin(mRandom));
                actor.AddComponent<Tint>(tint.R, tint.G, tint.B);
                actor.AddComponent<Lifetime>(lifetime(mRandom));
            }
            IP_CLIENT_TRACE("Spawned {} actors", spawned);
        }

        // Destroys every actor that has a Lifetime (everything but the player).
        void DestroySwarm()
        {
            ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
            ECS::ComponentView<Lifetime> lifetimes = components.GetAllComponents<Lifetime>();
            if (lifetimes.Empty()) return;

            // Same pattern as LifetimeSystem: collect first, destroy after the iteration.
            void* memory = mMemory.AllocateOnPerframe(sizeof(ECS::EntityID) * lifetimes.Size(), alignof(ECS::EntityID));
            if (!memory) return;

            ECS::EntityID* swarm = static_cast<ECS::EntityID*>(memory);
            size_t count = 0;
            for (const Lifetime& lifetime : lifetimes) swarm[count++] = lifetime.GetOwner();
            for (size_t index = 0; index < count; ++index)
            {
                ECS::Actor actor = ECS::GetActor(swarm[index]);
                ECS::DestroyActor(actor);
            }
            IP_CLIENT_DEBUG("Destroyed {} actors", count);
        }

        void TogglePause()
        {
            mPaused = !mPaused;
            ECS::SystemManager& systems = ECS::SystemManager::GetInstance();
            systems.GetSystem<MovementSystem>()->SetEnabled(!mPaused);
            systems.GetSystem<SpinSystem>()->SetEnabled(!mPaused);
            systems.GetSystem<LifetimeSystem>()->SetEnabled(!mPaused);
            IP_CLIENT_DEBUG("Simulation {}", mPaused ? "paused" : "resumed");
        }

        void UpdateStats(float deltaTime)
        {
            mElapsed += deltaTime;
            mStatsTime += deltaTime;
            ++mStatsFrames;
            if (mStatsTime >= 2.0f)
            {
                const ECS::ComponentManager& components = ECS::ComponentManager::GetInstance();
                IP_CLIENT_DEBUG("{:.1f} FPS | {} actors alive, {} transforms, {} lifetimes, {} destroyed so far",
                    mStatsFrames / mStatsTime,
                    ECS::EntityManager::GetInstance().GetAliveCount(),
                    components.GetComponentCount<Transform>(),
                    components.GetComponentCount<Lifetime>(),
                    mLifetimeSystem ? mLifetimeSystem->GetDestroyedCount() : 0);
                mStatsTime = 0.0f;
                mStatsFrames = 0;
            }
        }

    private:
        CoreEngine::MemoryManager mMemory;
        Color* mPalette = nullptr; // Lives in mMemory's stack allocator

        CoreEngine::IShader* mShader = nullptr;
        CoreEngine::IVertexBuffer* mTriangle = nullptr;

        ECS::Actor mPlayer;
        LifetimeSystem* mLifetimeSystem = nullptr; // Owned by the SystemManager
        std::mt19937 mRandom{std::random_device{}()};
        bool mPaused = false;

        float mElapsed = 0.0f;
        float mStatsTime = 0.0f;
        int mStatsFrames = 0;
    };
}

int main()
{
    try
    {
        CoreEngine::ApplicationConfiguration config{960, 640, "CoreEngine ECS Demo"};
        EcsDemo demo(config);
        demo.Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}
