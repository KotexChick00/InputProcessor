// Headless test program for the CoreEngine module (Core/).
//
// Covers: EventDispatcher, Logger, Time, Memory allocators, Wx input state machines
// and the Application loop (driven by an injected fake window + fake renderer, so no
// display or OpenGL context is needed).
//
// Usage:
//   test_core            run every test
//   test_core <filter>   run only tests whose name contains <filter> (e.g. "Memory")

#ifdef NDEBUG
    #undef NDEBUG
#endif

#include <Application/Application.hpp>
#include <Event/EventDispatcher.hpp>
#include <Input/Wx/WxKeyboardInput.hpp>
#include <Input/Wx/WxMouseInput.hpp>
#include <Logger/Logger.hpp>
#include <Memory/LinearAllocator.hpp>
#include <Memory/StackAllocator.hpp>
#include <Memory/PoolAllocator.hpp>
#include <Memory/MemoryManager.hpp>
#include <Memory/MemoryChunkManager.hpp>
#include <Time/Time.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using namespace CoreEngine;

// -----------------------------------------------------------------------------
// Minimal test harness
// -----------------------------------------------------------------------------

static int sChecks = 0;
static int sFailures = 0;

#define CHECK(expr)                                                                         \
    do                                                                                      \
    {                                                                                       \
        ++sChecks;                                                                          \
        if (!(expr))                                                                        \
        {                                                                                   \
            ++sFailures;                                                                    \
            std::cerr << "    FAILED: " #expr " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        }                                                                                   \
    } while (0)

#define CHECK_THROWS(ExceptionType, expr)                                       \
    do                                                                          \
    {                                                                           \
        bool thrown = false;                                                    \
        try                                                                     \
        {                                                                       \
            expr;                                                               \
        }                                                                       \
        catch (const ExceptionType&)                                            \
        {                                                                       \
            thrown = true;                                                      \
        }                                                                       \
        ++sChecks;                                                              \
        if (!thrown)                                                            \
        {                                                                       \
            ++sFailures;                                                        \
            std::cerr << "    FAILED: expected " #ExceptionType " from " #expr  \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";          \
        }                                                                       \
    } while (0)

static bool NearlyEqual(float a, float b, float epsilon = 1e-4f)
{
    return std::fabs(a - b) <= epsilon;
}

static bool IsAligned(const void* address, uintptr_t alignment)
{
    return (reinterpret_cast<uintptr_t>(address) & (alignment - 1)) == 0;
}

// -----------------------------------------------------------------------------
// EventDispatcher
// -----------------------------------------------------------------------------

static void TestEventDispatcher()
{
    EventDispatcher dispatcher;

    // Notifying an event type nobody listens to is a no-op.
    dispatcher.Notify(MouseScrollEventContext(1.0, 2.0));

    std::vector<std::string> calls;
    dispatcher.AddEventListener<WindowResizeEventContext>([&](const WindowResizeEventContext& e)
    {
        calls.push_back("resize:" + std::to_string(e.GetWidth()) + "x" + std::to_string(e.GetHeight()));
        return false;
    });
    dispatcher.AddEventListener<WindowResizeEventContext>([&](const WindowResizeEventContext&)
    {
        calls.push_back("resize-second");
        return false;
    });

    dispatcher.Notify(WindowResizeEventContext(640, 480));
    CHECK(calls.size() == 2);
    CHECK(calls.size() == 2 && calls[0] == "resize:640x480");
    CHECK(calls.size() == 2 && calls[1] == "resize-second");

    // A listener returning true stops propagation to the following listeners.
    int keyFirst = 0, keySecond = 0;
    dispatcher.AddEventListener<KeyBoardKeyEventContext>([&](const KeyBoardKeyEventContext& e)
    {
        ++keyFirst;
        return e.GetKey() == KeyboardKey::Escape;
    });
    dispatcher.AddEventListener<KeyBoardKeyEventContext>([&](const KeyBoardKeyEventContext&)
    {
        ++keySecond;
        return false;
    });

    dispatcher.Notify(KeyBoardKeyEventContext(KeyboardKey::A, KeyboardKeyState::Pressed));
    CHECK(keyFirst == 1 && keySecond == 1);
    dispatcher.Notify(KeyBoardKeyEventContext(KeyboardKey::Escape, KeyboardKeyState::Pressed));
    CHECK(keyFirst == 2 && keySecond == 1);

    // Events of different types are routed independently.
    calls.clear();
    dispatcher.Notify(MouseButtonEventContext(MouseButton::ButtonLeft, MouseButtonState::Pressed));
    CHECK(calls.empty());
    CHECK(keyFirst == 2);
}

// -----------------------------------------------------------------------------
// Logger
// -----------------------------------------------------------------------------

struct LogRecord
{
    LogLevel Level;
    std::string Message;
};

class CaptureLogger : public ILoggerImplentation
{
public:
    explicit CaptureLogger(std::vector<LogRecord>* records)
        : mRecords(records)
    {
    }

    void Log(LogLevel level, std::string_view message, const std::source_location&) override
    {
        mRecords->push_back({level, std::string(message)});
    }

private:
    std::vector<LogRecord>* mRecords;
};

static void TestLogger()
{
    // Logging without an implementation must be a silent no-op.
    Logger::SetEngineImplementation(nullptr);
    Logger::SetClientImplementation(nullptr);
    IP_ENGINE_ERROR("nobody listens {}", 1);
    IP_CLIENT_ERROR("nobody listens {}", 2);

    std::vector<LogRecord> engineRecords;
    std::vector<LogRecord> clientRecords;
    Logger::SetEngineImplementation(new CaptureLogger(&engineRecords));
    Logger::SetClientImplementation(new CaptureLogger(&clientRecords));

    IP_ENGINE_WARN("value = {}, name = {}", 42, "core");
    IP_ENGINE_CRITICAL("fatal");
    IP_CLIENT_ERROR("client {}", 3.5);

    CHECK(engineRecords.size() == 2);
    CHECK(clientRecords.size() == 1);
    if (engineRecords.size() == 2)
    {
        CHECK(engineRecords[0].Level == LogLevel::Warning);
        CHECK(engineRecords[0].Message == "value = 42, name = core");
        CHECK(engineRecords[1].Level == LogLevel::Critical);
    }
    if (clientRecords.size() == 1)
    {
        CHECK(clientRecords[0].Level == LogLevel::Error);
        CHECK(clientRecords[0].Message == "client 3.5");
    }

    // TRACE/DEBUG are active in this build because NDEBUG is undefined.
    IP_ENGINE_TRACE("trace");
    IP_ENGINE_DEBUG("debug");
    CHECK(engineRecords.size() == 4);

    Logger::SetEngineImplementation(nullptr);
    Logger::SetClientImplementation(nullptr);
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

static void TestTime()
{
    Time* time = Time::GetInstance();
    CHECK(time == Time::GetInstance());

    time->SetTimeNow(12.5f);
    time->SetDeltaTime(0.016f);
    time->SetTimeScale(0.5f);
    CHECK(NearlyEqual(time->GetTimeNow(), 12.5f));
    CHECK(NearlyEqual(time->GetDeltaTime(), 0.016f));
    CHECK(NearlyEqual(time->GetTimeScale(), 0.5f));

    time->SetTimeScale(1.0f);
}

// -----------------------------------------------------------------------------
// Memory allocators
// -----------------------------------------------------------------------------

static void TestLinearAllocator()
{
    LinearAllocator allocator(256);

    void* first = allocator.Allocate(10, 16);
    void* second = allocator.Allocate(32, 16);
    void* third = allocator.Allocate(4, 4);
    CHECK(first && second && third);
    CHECK(IsAligned(first, 16));
    CHECK(IsAligned(second, 16));
    CHECK(IsAligned(third, 4));

    // Allocations never overlap and grow upwards.
    auto a = reinterpret_cast<uintptr_t>(first);
    auto b = reinterpret_cast<uintptr_t>(second);
    auto c = reinterpret_cast<uintptr_t>(third);
    CHECK(b >= a + 10);
    CHECK(c >= b + 32);

    // Memory is writable across the whole block.
    std::memset(second, 0xAB, 32);
    CHECK(static_cast<unsigned char*>(second)[31] == 0xAB);

    // Requests larger than the remaining space fail without corrupting state.
    CHECK(allocator.Allocate(1024, 8) == nullptr);
    CHECK(allocator.Allocate(8, 8) != nullptr);

    // Clear resets the allocator to its first address.
    allocator.Clear();
    CHECK(allocator.Allocate(10, 16) == first);
}

static void TestStackAllocator()
{
    StackAllocator allocator(512);

    void* bottom = allocator.Allocate(32, 8);
    void* top = allocator.Allocate(16, 16);
    CHECK(bottom && top);
    CHECK(IsAligned(bottom, 8));
    CHECK(IsAligned(top, 16));
    CHECK(reinterpret_cast<uintptr_t>(top) >= reinterpret_cast<uintptr_t>(bottom) + 32);

    // LIFO: freeing the top and allocating the same shape reuses the same slot.
    allocator.Free(top);
    void* topAgain = allocator.Allocate(16, 16);
    CHECK(topAgain == top);

    // Unwinding the whole stack returns to the bottom slot.
    allocator.Free(topAgain);
    allocator.Free(bottom);
    CHECK(allocator.Allocate(32, 8) == bottom);

    // Out of memory returns nullptr.
    CHECK(allocator.Allocate(4096, 8) == nullptr);

    allocator.Clear();
    CHECK(allocator.Allocate(32, 8) == bottom);
}

static void TestMemoryManager()
{
    MemoryConfiguration config;
    config.LinearAllocateSize = 256;
    config.StackAllocateSize = 512;
    MemoryManager manager(config);

    // Per-frame memory is reset by Update().
    void* frameMemory = manager.AllocateOnPerframe(32, 16);
    CHECK(frameMemory && IsAligned(frameMemory, 16));
    manager.Update();
    CHECK(manager.AllocateOnPerframe(32, 16) == frameMemory);

    // LIFO frees unwind the stack completely.
    void* a = manager.AllocateOnStack(32, 8);
    void* b = manager.AllocateOnStack(16, 16);
    CHECK(a && b);
    manager.FreeOnStack(b);
    manager.FreeOnStack(a);
    manager.DetectMemoryLeak(); // no warning
    CHECK(manager.AllocateOnStack(32, 8) == a);
    manager.FreeOnStack(a);

    // Out-of-order free is deferred until everything above it is freed.
    a = manager.AllocateOnStack(32, 8);
    b = manager.AllocateOnStack(16, 16);
    void* c = manager.AllocateOnStack(24, 8);
    CHECK(a && b && c);
    manager.FreeOnStack(b);                       // deferred: c is still alive
    manager.DetectMemoryLeak(); // warns: a and c (b is pending)
    void* d = manager.AllocateOnStack(8, 8);
    CHECK(reinterpret_cast<uintptr_t>(d) >= reinterpret_cast<uintptr_t>(c) + 24);
    manager.FreeOnStack(d);
    manager.FreeOnStack(c);                       // releases c, then pending b
    manager.DetectMemoryLeak(); // warns: a
    CHECK(manager.AllocateOnStack(16, 16) == b);  // b's slot is free again
    manager.FreeOnStack(b);
    manager.FreeOnStack(a);
    manager.DetectMemoryLeak(); // no warning

    // Unfreed allocation is reported, ClearOnStack drops it.
    CHECK(manager.AllocateOnStack(64, 8) == a);
    manager.DetectMemoryLeak(); // warns: a
    manager.ClearOnStack();
    manager.DetectMemoryLeak(); // no warning

    // Clear resets both allocators.
    manager.AllocateOnPerframe(16, 8);
    manager.AllocateOnStack(16, 8);
    manager.Clear();
    manager.DetectMemoryLeak(); // no warning
    CHECK(manager.AllocateOnPerframe(32, 16) == frameMemory);
    CHECK(manager.AllocateOnStack(32, 8) == a);
    manager.FreeOnStack(a);
}

static void TestMemoryChunkManager()
{
    MemoryChunkManagerConfiguration config;
    config.StackAllocateSize = 1024;
    config.BlockSize = 24;
    config.MaxBlockCountPerChunk = 2;
    config.BlockAlignment = 16;
    MemoryChunkManager manager(config);
    CHECK(manager.GetChunkCount() == 0);

    // The first chunk is created on demand and holds MaxBlockCountPerChunk blocks.
    void* first = manager.Allocate();
    void* second = manager.Allocate();
    CHECK(first && second && first != second);
    CHECK(IsAligned(first, 16) && IsAligned(second, 16));
    CHECK(manager.GetChunkCount() == 1);

    // A full chunk makes the manager create a new one.
    void* third = manager.Allocate();
    void* fourth = manager.Allocate();
    CHECK(third && fourth && third != fourth);
    CHECK(third != first && third != second);
    CHECK(manager.GetChunkCount() == 2);

    // Blocks are writable without touching their neighbours.
    void* blocks[] = {first, second, third, fourth};
    for (size_t i = 0; i < 4; ++i) std::memset(blocks[i], static_cast<int>(0x20 + i), 24);
    for (size_t i = 0; i < 4; ++i) CHECK(static_cast<unsigned char*>(blocks[i])[23] == 0x20 + i);

    // Free finds the owning chunk, and the freed block is reused before a new chunk is made.
    manager.Free(first);
    CHECK(manager.Allocate() == first);
    CHECK(manager.GetChunkCount() == 2);

    // Unknown addresses and nullptr are ignored (an error is logged for the former).
    int notOwned = 0;
    manager.Free(&notOwned);
    manager.Free(nullptr);

    // Freeing a block of the second chunk goes to that chunk.
    manager.Free(fourth);
    CHECK(manager.Allocate() == fourth);
    CHECK(manager.GetChunkCount() == 2);

    // Clear destroys every chunk; new chunks are created again on demand.
    manager.Clear();
    CHECK(manager.GetChunkCount() == 0);
    CHECK(manager.Allocate() != nullptr);
    CHECK(manager.GetChunkCount() == 1);

    // When the stack can't hold another chunk, Allocate returns nullptr.
    MemoryChunkManagerConfiguration smallConfig = config;
    smallConfig.StackAllocateSize = 128; // room for one MemoryChunk object only (x64)
    MemoryChunkManager smallManager(smallConfig);
    CHECK(smallManager.Allocate() && smallManager.Allocate());
    CHECK(smallManager.Allocate() == nullptr);
    CHECK(smallManager.GetChunkCount() == 1);
}

static void TestPoolAllocator()
{
    constexpr size_t BlockCount = 4;
    PoolAllocator pool(24, BlockCount, 16);
    CHECK(pool.GetBlockSize() == 24);
    CHECK(pool.GetBlockCount() == BlockCount);
    CHECK(pool.GetFreeBlockCount() == BlockCount);

    // Every block is distinct, aligned, ascending and at least one stride (32 bytes) apart.
    void* blocks[BlockCount] = {};
    for (size_t i = 0; i < BlockCount; ++i)
    {
        blocks[i] = pool.Allocate(24, 16);
        CHECK(blocks[i] != nullptr);
        CHECK(IsAligned(blocks[i], 16));
        if (i > 0) CHECK(reinterpret_cast<uintptr_t>(blocks[i]) >= reinterpret_cast<uintptr_t>(blocks[i - 1]) + 32);
    }
    CHECK(pool.GetFreeBlockCount() == 0);

    // Blocks are fully writable without touching their neighbours.
    for (size_t i = 0; i < BlockCount; ++i) std::memset(blocks[i], static_cast<int>(0x10 + i), 24);
    for (size_t i = 0; i < BlockCount; ++i)
    {
        CHECK(static_cast<unsigned char*>(blocks[i])[0] == 0x10 + i);
        CHECK(static_cast<unsigned char*>(blocks[i])[23] == 0x10 + i);
    }

    // Exhausted pool returns nullptr; smaller requests and weaker alignments still fit a block.
    CHECK(pool.Allocate(8, 8) == nullptr);

    // Blocks can be freed in any order; the most recently freed block is reused first.
    pool.Free(blocks[1]);
    pool.Free(blocks[3]);
    CHECK(pool.GetFreeBlockCount() == 2);
    CHECK(pool.Allocate(8, 4) == blocks[3]);
    CHECK(pool.Allocate(24, 16) == blocks[1]);
    CHECK(pool.GetFreeBlockCount() == 0);

    // Freeing nullptr is a no-op.
    pool.Free(nullptr);
    CHECK(pool.GetFreeBlockCount() == 0);

    // Clear returns every block, handed out again from the first one.
    pool.Clear();
    CHECK(pool.GetFreeBlockCount() == BlockCount);
    CHECK(pool.Allocate(1, 1) == blocks[0]);

    // Tiny blocks still hold the free-list pointer, and the default alignment is respected.
    PoolAllocator tinyPool(1, 3);
    void* a = tinyPool.Allocate(1, 1);
    void* b = tinyPool.Allocate(1, 1);
    CHECK(a && b && a != b);
    CHECK(IsAligned(a, alignof(std::max_align_t)) && IsAligned(b, alignof(std::max_align_t)));
    CHECK(reinterpret_cast<uintptr_t>(b) - reinterpret_cast<uintptr_t>(a) >= sizeof(void*));

    // Contains only accepts addresses inside the allocator's own memory.
    int notOwned = 0;
    CHECK(pool.Contains(blocks[0]) && pool.Contains(blocks[BlockCount - 1]));
    CHECK(tinyPool.Contains(a) && !pool.Contains(a));
    CHECK(!pool.Contains(&notOwned));
}

// -----------------------------------------------------------------------------
// Input (wx backends are pure state machines, no wx event loop required)
// -----------------------------------------------------------------------------

static void TestWxKeyboardInput()
{
    WxKeyboardInput keyboard;
    CHECK(keyboard.GetKeyState(KeyboardKey::A) == KeyState::None);
    CHECK(keyboard.CheckIsReleased(KeyboardKey::A));

    keyboard.NotifyKeyDown('A');
    CHECK(keyboard.CheckIsPressed(KeyboardKey::A));
    CHECK(keyboard.CheckIsJustPressed(KeyboardKey::A));
    CHECK(keyboard.GetKeyState(KeyboardKey::A) == KeyState::Pressed);
    CHECK(!keyboard.CheckIsPressed(KeyboardKey::B));

    keyboard.Update(); // next frame: still held, no longer "just" pressed
    CHECK(keyboard.CheckIsPressed(KeyboardKey::A));
    CHECK(!keyboard.CheckIsJustPressed(KeyboardKey::A));

    keyboard.NotifyKeyUp('A');
    CHECK(!keyboard.CheckIsPressed(KeyboardKey::A));
    CHECK(keyboard.GetKeyState(KeyboardKey::A) == KeyState::Released);

    keyboard.Update(); // Released lasts a single frame
    CHECK(keyboard.GetKeyState(KeyboardKey::A) == KeyState::None);

    // Digits map too, unknown codes are ignored.
    keyboard.NotifyKeyDown('7');
    CHECK(keyboard.CheckIsPressed(KeyboardKey::Seven));
    keyboard.NotifyKeyDown(0x7FFFFF);
    CHECK(!keyboard.CheckIsPressed(KeyboardKey::Unknow));
}

static void TestWxMouseInput()
{
    WxMouseInput mouse;
    CHECK(mouse.GetMouseButtonState(MouseButton::ButtonLeft) == MouseButtonState::None);
    CHECK(NearlyEqual(mouse.GetPositionX(), 0.0f) && NearlyEqual(mouse.GetPositionY(), 0.0f));

    mouse.NotifyButtonDown(MouseButton::ButtonRight);
    mouse.SetPosition(100.0f, 50.0f);
    mouse.SetDelta(3.0f, -4.0f);
    mouse.SetScroll(0.0f, 1.0f);
    CHECK(mouse.CheckIsPressed(MouseButton::ButtonRight));
    CHECK(mouse.CheckIsJustPressed(MouseButton::ButtonRight));
    CHECK(mouse.GetMouseButtonState(MouseButton::ButtonRight) == MouseButtonState::Pressed);
    CHECK(!mouse.CheckIsPressed(MouseButton::ButtonLeft));

    mouse.Update(); // deltas and scroll are per-frame, position persists
    CHECK(!mouse.CheckIsJustPressed(MouseButton::ButtonRight));
    CHECK(mouse.CheckIsPressed(MouseButton::ButtonRight));
    CHECK(NearlyEqual(mouse.GetDeltaX(), 0.0f) && NearlyEqual(mouse.GetDeltaY(), 0.0f));
    CHECK(NearlyEqual(mouse.GetScrollY(), 0.0f));
    CHECK(NearlyEqual(mouse.GetPositionX(), 100.0f) && NearlyEqual(mouse.GetPositionY(), 50.0f));

    mouse.NotifyButtonUp(MouseButton::ButtonRight);
    CHECK(mouse.GetMouseButtonState(MouseButton::ButtonRight) == MouseButtonState::Released);
    mouse.Update();
    CHECK(mouse.GetMouseButtonState(MouseButton::ButtonRight) == MouseButtonState::None);

    // ButtonLeft/Right/Middle are aliases of Button1/2/3.
    mouse.NotifyButtonDown(MouseButton::Button3);
    CHECK(mouse.CheckIsPressed(MouseButton::ButtonMiddle));
}

// -----------------------------------------------------------------------------
// Application loop with injected fake window / renderer
// -----------------------------------------------------------------------------

// Observations survive the fakes: Application owns (and deletes) injected objects.
struct ApplicationProbe
{
    std::vector<std::string> Lifecycle;
    WindowConfiguration InitConfig{};
    int SwapCount = 0;
    int EndFrameCount = 0;
    int ConfigCalls = 0;
    RendererConfiguration LastRendererConfig{};
};

class FakeRendererCommand : public IRendererCommand
{
public:
    void ClearBuffers(ClearBufferMasks) override { }
    void Draw(RenderMode, unsigned int, unsigned int) override { }
    void DrawIndex(RenderMode, unsigned int, unsigned int) override { }
    void SetPolygonMode(PolygonMode) override { }
};

class FakeRenderer : public IRenderer
{
public:
    explicit FakeRenderer(ApplicationProbe* probe)
        : mProbe(probe)
    {
    }

    ~FakeRenderer() override { mProbe->Lifecycle.push_back("renderer-destroyed"); }

    void Config(RendererConfiguration configuration) override
    {
        mConfig = configuration;
        mProbe->LastRendererConfig = configuration;
        ++mProbe->ConfigCalls;
    }

    IRendererCommand* GetRendererCommand() override { return &mCommand; }
    IResourceManager* GetResourceManager() override { return nullptr; }
    IFrameBufferManager* GetFrameBufferManager() override { return nullptr; }
    void Free() override { }
    RendererConfiguration GetConfig() override { return mConfig; }

private:
    ApplicationProbe* mProbe;
    RendererConfiguration mConfig{};
    FakeRendererCommand mCommand;
};

class FakeWindow : public IWindow
{
public:
    explicit FakeWindow(ApplicationProbe* probe)
        : mProbe(probe)
    {
    }

    ~FakeWindow() override { mProbe->Lifecycle.push_back("window-destroyed"); }

    void Init(const WindowConfiguration& config) override
    {
        mProbe->InitConfig = config;
        mProbe->Lifecycle.push_back("window-init");
    }

    // Frame 1 emits one of every raw window event; frame 2 presses a key through the polling path.
    void PollEvents() override
    {
        ++mFrame;
        if (mFrame == 1)
        {
            if (mResize) mResize({800, 600});
            if (mKeyboard) mKeyboard({WindowKeyboardKey::A, WindowKeyboardKeyState::Pressed});
            if (mMouseButton) mMouseButton({WindowMouseButton::Button2, WindowMouseButtonState::Released});
            if (mScroll) mScroll({1.5, -2.0});
            if (mMouseMove) mMouseMove({10.0, 20.0});
        }
        else if (mFrame == 2)
        {
            mKeyboardInput.NotifyKeyDown('W');
        }
    }

    void SwapBuffers() override { ++mProbe->SwapCount; }
    bool CheckShouldClose() override { return mClosed; }

    void Close() override
    {
        mClosed = true;
        mProbe->Lifecycle.push_back("window-close");
    }

    void OnMouseMoveEventCallback(std::function<void(const WindowMouseMoveEventContext&)> callback) override { mMouseMove = callback; }
    void OnMouseButtonEventCallback(std::function<void(const WindowMouseButtonEventContext&)> callback) override { mMouseButton = callback; }
    void OnKeyboardEventCallback(std::function<void(const WindowKeyboardKeyEventContext&)> callback) override { mKeyboard = callback; }
    void OnMouseSrollEventCallback(std::function<void(const WindowMouseScrollEventContext&)> callback) override { mScroll = callback; }
    void OnWindowReiszeEventCallback(std::function<void(const WindowSizeEventContext&)> callback) override { mResize = callback; }

    bool ShowOpenFileDialog(const WindowFileDialogOptions& options, std::string& outPath) override
    {
        (void)options;
        outPath.clear();
        return false; // The injected fake window never shows a real dialog.
    }

    void OnOpenFileRequestCallback(std::function<void()> callback) override { mOpenFileRequest = callback; }

    InputState GetInput() const override
    {
        return {const_cast<WxKeyboardInput*>(&mKeyboardInput), const_cast<WxMouseInput*>(&mMouseInput)};
    }

    // Deterministic clock: every query advances 50 ms, so the FPS limiter never sleeps.
    float GetCurrentSeconds() override
    {
        mClock += 0.05f;
        return mClock;
    }

    void Accept(IWindowVisitor*) override { }

    void EndFrame() override
    {
        ++mProbe->EndFrameCount;
        mKeyboardInput.Update();
        mMouseInput.Update();
    }

private:
    ApplicationProbe* mProbe;
    int mFrame = 0;
    bool mClosed = false;
    float mClock = 0.0f;
    WxKeyboardInput mKeyboardInput;
    WxMouseInput mMouseInput;

    std::function<void(const WindowMouseMoveEventContext&)> mMouseMove;
    std::function<void(const WindowMouseButtonEventContext&)> mMouseButton;
    std::function<void(const WindowKeyboardKeyEventContext&)> mKeyboard;
    std::function<void(const WindowMouseScrollEventContext&)> mScroll;
    std::function<void(const WindowSizeEventContext&)> mResize;
    std::function<void()> mOpenFileRequest;
};

class TestApplication : public Application
{
public:
    TestApplication(ApplicationConfiguration& config, ApplicationProbe* probe)
        : Application(config)
        , mProbe(probe)
    {
    }

    int InitCalls = 0;
    int LoopFrames = 0;
    int ShutdownCalls = 0;
    std::vector<std::string> Events;
    bool SawPolledKey = false;
    bool SawJustPressed = false;
    bool RendererAvailable = false;
    float FirstDeltaTime = -1.0f;

    void OnInitClient() override
    {
        ++InitCalls;
        RendererAvailable = GetRenderer() != nullptr && GetRenderer()->GetRendererCommand() != nullptr;

        auto* dispatcher = GetEventDispatcher();
        dispatcher->AddEventListener<WindowResizeEventContext>([this](const WindowResizeEventContext& e)
        {
            Events.push_back("resize " + std::to_string(e.GetWidth()) + "x" + std::to_string(e.GetHeight()));
            return false;
        });
        dispatcher->AddEventListener<KeyBoardKeyEventContext>([this](const KeyBoardKeyEventContext& e)
        {
            bool ok = e.GetKey() == KeyboardKey::A && e.GetState() == KeyboardKeyState::Pressed;
            Events.push_back(ok ? "key A pressed" : "key unexpected");
            return false;
        });
        dispatcher->AddEventListener<MouseButtonEventContext>([this](const MouseButtonEventContext& e)
        {
            bool ok = e.GetButton() == MouseButton::ButtonRight && e.GetButtonState() == MouseButtonState::Released;
            Events.push_back(ok ? "mouse right released" : "mouse unexpected");
            return false;
        });
        dispatcher->AddEventListener<MouseScrollEventContext>([this](const MouseScrollEventContext& e)
        {
            Events.push_back("scroll " + std::to_string(e.GetDeltaX()) + " " + std::to_string(e.GetDeltaY()));
            return false;
        });
    }

    void OnLoopClient() override
    {
        ++LoopFrames;
        if (LoopFrames == 1) FirstDeltaTime = GetTime()->GetDeltaTime();

        const InputState& input = GetInput();
        if (input.KeyboardInput && input.KeyboardInput->CheckIsPressed(KeyboardKey::W))
        {
            SawPolledKey = true;
            if (LoopFrames == 2) SawJustPressed = input.KeyboardInput->CheckIsJustPressed(KeyboardKey::W);
        }

        if (LoopFrames == 3) RequestClose();
    }

    void OnShutdownClient() override
    {
        ++ShutdownCalls;
        mProbe->Lifecycle.push_back("client-shutdown");
    }

private:
    ApplicationProbe* mProbe;
};

static ApplicationConfiguration MakeInjectedConfiguration(ApplicationProbe* probe)
{
    ApplicationConfiguration config{320, 240, "Core Test"};
    config.MaxFPS = 60;
    config.WindowPlatformSpec = WindowPlatformSpec::Injection;
    config.InjectedWindow = new FakeWindow(probe);
    config.RenderAPI = RenderAPI::Injection;
    config.InjectedRenderer = new FakeRenderer(probe);
    return config;
}

static void TestApplicationLoop()
{
    ApplicationProbe probe;
    ApplicationConfiguration config = MakeInjectedConfiguration(&probe);
    TestApplication app(config, &probe);
    app.Run();

    // Window received the configured size/title.
    CHECK(probe.InitConfig.Width == 320 && probe.InitConfig.Height == 240);
    CHECK(probe.InitConfig.Title == "Core Test");

    // Loop ran exactly until RequestClose(), with one EndFrame/SwapBuffers per completed frame.
    CHECK(app.InitCalls == 1);
    CHECK(app.LoopFrames == 3);
    CHECK(app.ShutdownCalls == 1);
    CHECK(probe.EndFrameCount == 3);
    CHECK(probe.SwapCount == 3);
    CHECK(app.RendererAvailable);

    // Delta time comes from the window clock (one 50 ms tick per BeginLoop; 50 ms > 1/60 s, so no wait).
    CHECK(NearlyEqual(app.FirstDeltaTime, 0.05f, 1e-3f));

    // Raw window events are translated into engine events, in emission order.
    std::vector<std::string> expected = {"resize 800x600", "key A pressed", "mouse right released",
                                         "scroll " + std::to_string(1.5) + " " + std::to_string(-2.0)};
    CHECK(app.Events == expected);

    // Polled input flows through GetInput(), and EndFrame advances the state machine.
    CHECK(app.SawPolledKey);
    CHECK(app.SawJustPressed);

    // Initial viewport = window size; resize events update the renderer viewport.
    CHECK(probe.ConfigCalls >= 2);
    CHECK(probe.LastRendererConfig.ViewPortOptions.Width == 800);
    CHECK(probe.LastRendererConfig.ViewPortOptions.Height == 600);

    // Shutdown order: client hook, renderer released before the window is closed, then window freed.
    std::vector<std::string> lifecycle = {"window-init", "client-shutdown", "renderer-destroyed", "window-close", "window-destroyed"};
    CHECK(probe.Lifecycle == lifecycle);
}

static void TestApplicationRejectsInvalidConfiguration()
{
    ApplicationProbe probe;

    ApplicationConfiguration zeroSize{0, 240, "Invalid"};
    zeroSize.WindowPlatformSpec = WindowPlatformSpec::Injection;
    zeroSize.RenderAPI = RenderAPI::Injection;
    TestApplication zeroSizeApp(zeroSize, &probe);
    CHECK_THROWS(std::invalid_argument, zeroSizeApp.Run());

    ApplicationConfiguration noTitle{320, 240, nullptr};
    noTitle.WindowPlatformSpec = WindowPlatformSpec::Injection;
    noTitle.RenderAPI = RenderAPI::Injection;
    TestApplication noTitleApp(noTitle, &probe);
    CHECK_THROWS(std::invalid_argument, noTitleApp.Run());

    ApplicationConfiguration zeroFps{320, 240, "Invalid"};
    zeroFps.MaxFPS = 0;
    zeroFps.WindowPlatformSpec = WindowPlatformSpec::Injection;
    zeroFps.RenderAPI = RenderAPI::Injection;
    TestApplication zeroFpsApp(zeroFps, &probe);
    CHECK_THROWS(std::invalid_argument, zeroFpsApp.Run());

    // Validation happens before any client hook or window work.
    CHECK(zeroSizeApp.InitCalls == 0 && noTitleApp.InitCalls == 0 && zeroFpsApp.InitCalls == 0);
    CHECK(probe.Lifecycle.empty());
}

// -----------------------------------------------------------------------------
// Entry point
// -----------------------------------------------------------------------------

struct TestCase
{
    const char* Name;
    void (*Run)();
};

int main(int argc, char** argv)
{
    const TestCase tests[] = {
        {"EventDispatcher", TestEventDispatcher},
        {"Logger", TestLogger},
        {"Time", TestTime},
        {"LinearAllocator", TestLinearAllocator},
        {"StackAllocator", TestStackAllocator},
        {"PoolAllocator", TestPoolAllocator},
        {"MemoryManager", TestMemoryManager},
        {"MemoryChunkManager", TestMemoryChunkManager},
        {"WxKeyboardInput", TestWxKeyboardInput},
        {"WxMouseInput", TestWxMouseInput},
        {"ApplicationLoop", TestApplicationLoop},
        {"ApplicationRejectsInvalidConfiguration", TestApplicationRejectsInvalidConfiguration},
    };

    std::string_view filter = argc > 1 ? argv[1] : "";
    int ran = 0;
    for (const TestCase& test : tests)
    {
        if (!filter.empty() && std::string_view(test.Name).find(filter) == std::string_view::npos) continue;

        int failuresBefore = sFailures;
        std::cout << "[ RUN  ] " << test.Name << "\n";
        try
        {
            test.Run();
        }
        catch (const std::exception& e)
        {
            ++sFailures;
            std::cerr << "    FAILED: unexpected exception: " << e.what() << "\n";
        }
        std::cout << (sFailures == failuresBefore ? "[  OK  ] " : "[ FAIL ] ") << test.Name << "\n";
        ++ran;
    }

    if (ran == 0)
    {
        std::cerr << "No test matches filter '" << filter << "'\n";
        return 1;
    }

    std::cout << "\n" << ran << " test(s), " << sChecks << " check(s), " << sFailures << " failure(s)\n";
    return sFailures == 0 ? 0 : 1;
}
