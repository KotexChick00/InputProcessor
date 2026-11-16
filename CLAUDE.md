# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & test

C++20 (requires `std::format`: MSVC from VS 2022+, or GCC 13+), CMake ≥ 3.21, dependencies from `vcpkg.json` (wxWidgets, Assimp, spdlog, glm, stb). On Windows, run from a Developer PowerShell / x64 Native Tools prompt (the presets force `cl.exe` + Ninja), with `VCPKG_ROOT` set.

```sh
cmake --preset vcpkg-debug            # Linux / system libs: --preset debug
cmake --build --preset vcpkg-debug
ctest --preset vcpkg-debug            # run all tests
ctest --preset vcpkg-debug -R core             # Core/ only, headless (no display/GL)
build/vcpkg-debug/test_core Memory             # one group: name filter (substring)
ctest --preset vcpkg-debug -R engine_logic     # run one test executable
ctest --preset vcpkg-debug -R wx_integration   # needs a display + OpenGL 3.3
```

- On a Linux CI without a display: `GDK_BACKEND=x11 xvfb-run -a ctest --preset debug`.
- Tests have no framework. Each of `tests/test_engine_logic.cpp` and `tests/test_wx_integration.cpp` is its own `main()` that calls `TestXxx()` functions using `assert` (NDEBUG is force-disabled). To run a single group, temporarily comment out the other calls in `main()`. Fixtures live in `tests/assets` (`TEST_ASSET_DIR`).
- `CMakeLists.txt` uses `file(GLOB ...)` → adding a new `.cpp` only requires re-running configure. **Only Wx/OpenGL/SpdLog sources are compiled**; `Core/Input/GLFW`, `Core/Window/GLFW` and `Core/UI/Imgui` are not in the CMake build (they are still listed in the legacy `ObjFileInutProcessor.vcxproj`). When adding or removing headers, update the `.vcxproj`/`.vcxproj.filters` too if you want the `.slnx` project to stay in sync.
- The README mentions an `InputProcessor` target (`examples/gltf_viewer.cpp`, `--frames N` option), but the current `CMakeLists.txt` only defines `engine`, `DomainDemo` (`main.cpp`) and the two tests. Check before you rely on it.

## Core architecture (`Core/`)

All of the engine lives in **a single namespace, `CoreEngine`** (no sub-namespaces). Implementations are distinguished by the class-name prefix (`Opengl*`, `Wx*`, `GLFW*`, `Imgui*`, `SpdLog*`), not by namespace. Include paths are rooted at `Core/` with angle brackets: `#include <Window/IWindow.hpp>`. `Domain/` (camera, model) is also on the include path, but uses its own namespaces (`Domain::Camera::...`, `Model`).

- **Application** (`Application/`): the client subclasses `CoreEngine::Application`, overrides `OnInitClient/OnLoopClient/OnShutdownClient`, and calls `Run()`. `ApplicationConfiguration` picks the window platform (`WindowPlatformSpec`) and the render API (`RenderAPI`); both have an `Injection` value for supplying a ready-made `IWindow*`/`IRenderer*` (tests use this). Loop: `BeginLoop` (caps FPS, updates `Time`, `PollEvents`) → `OnLoopClient` → `EndLoop` (`EndFrame` resets the per-frame input state, then `SwapBuffers`).
- **Window** (`Window/`): `IWindow` is the platform abstraction. It reports **raw** events through callbacks using its own types (`WindowKeyboardKey`, `WindowMouseButton`, `WindowSizeEventContext`, ...). `WindowPlatformFactory` creates the implementation (`WxWindow` is the default). `IWindowVisitor` (visitor pattern) lets the UI/ImGui layer reach the concrete window without casting.
- **Two input paths** that both come from the window:
  1. *Polling*: `IWindow::GetInput()` → `InputState { KeyboardInput*, MouseInput* }` (`Input/`), read via `Application::GetInput()` (returns a reference to the application-owned copy; controllers keep that reference).
  2. *Events*: `Application::SetupEvents` translates the `Window*` types into the shared input types (`Input/InputCodes.hpp`: `KeyboardKey`, `MouseButton`, `*State`) and calls `EventDispatcher::Notify` with the classes in `Event/EventContext.hpp` (`WindowResizeEventContext`, `KeyBoardKeyEventContext`, ...). Clients subscribe with `GetEventDispatcher()->AddEventListener<T>(callback)`. Dispatch is keyed by `typeid(T).name()`, and a callback returning `true` stops propagation.
  When adding a new key or button, update all three enums: `WindowKeyboardKey` (IWindow.hpp), `KeyboardKey` (InputCodes.hpp), and the mapping functions in `Application.cpp` plus each window/input backend.
- **Renderer** (`Renderer/`): the `IRenderer` facade exposes `IRendererCommand` (draw/clear/state), `IResourceManager` (creates and owns buffers/shaders/textures/cubemaps, looked up by id) and `IFrameBufferManager`. State is configured with `RendererConfiguration` (`RendererPrimitives.hpp`). `RendererFactory` → `OpenglRenderer` (a singleton). GPU resources must be released while the GL context is still current (`Application::Shutdown` resets the renderer before closing the window).
- **Logger** (`Logger/`): always log through the macros `IP_ENGINE_*` (engine) / `IP_CLIENT_*` (client), never by calling `Logger` directly. TRACE/DEBUG compile to nothing in Release. The backend is set up via `LoggerFactory` (SpdLog).
- **Time** (`Time/`): the singleton `Time::GetInstance()`, accessed via `Application::GetTime()`.
- **Memory** (`Memory/`): `LinearAllocator` (bump, `Clear()` only), `StackAllocator` (LIFO `Free`), `PoolAllocator` (fixed-size blocks, O(1) `Free` in any order), all derived from `MemoryAllocator`. `Core/Memory/*.cpp` is compiled into `engine`.
- `pch.h`: common STL includes, the aliases `Unique<T>` (= `std::unique_ptr`) and `Shared<T>`, and the macros `CORE_API` (export/import), `CORE_FORCE_INLINE`, `CORE_ASSERT`, `CORE_FREE`.

## Coding rules (C# style)

`Core/` follows these rules; the formatting part is encoded in `Core/.clang-format`. After editing a file in `Core/`, run clang-format on it (Visual Studio ships it at `VC/Tools/Llvm/x64/bin/clang-format.exe`):

```sh
clang-format -i --style=file <file>
```

clang-format does **not** reformat the bodies of lambdas passed as arguments (e.g. the callbacks in `Application::SetupEvents`, `GLFWWindow::Init`): write those blocks in Allman style by hand. `SortIncludes` is off because the wx/glad include order matters.

**Naming**
- `PascalCase` for: classes, structs, enums and enum values, **functions/methods** (including private ones), public struct fields (`Width`, `KeyboardInput`), type aliases.
- Interfaces start with `I`: `IWindow`, `IRenderer`, `IResourceManager`.
- Private/protected member variables: `m` + PascalCase (`mWindow`, `mConfig`). Static members: `s` + PascalCase (`sInstance`).
- Local variables and parameters: `camelCase` (`windowConfig`, `deltaTime`).
- Macros: `UPPER_SNAKE_CASE` with a prefix (`CORE_*`, `IP_*`).
- Implementation classes are named `<Backend><Concept>`: `OpenglTexture`, `WxKeyboardInput`.

**Formatting**
- Indent with **4 spaces**, no tabs.
- **Allman braces**: the opening brace `{` goes on its own line for namespaces, classes/structs/enums, functions, and control statements (`if/else/for/while/switch/try`). `else`/`catch` start a new line after `}`.
  ```cpp
  namespace CoreEngine
  {
      void Application::SetupWindow()
      {
          if (mConfig.WindowPlatformSpec == WindowPlatformSpec::Injection)
          {
              mWindow.reset(mConfig.InjectedWindow);
          }
          else
          {
              mWindow.reset(WindowPlatformFactory::Create(mConfig.WindowPlatformSpec));
          }
      }
  }
  ```
- Exceptions allowed on one line: empty bodies `{ }`, and short getters/setters in headers (`CORE_FORCE_INLINE float GetWidth() const { return mWidth; }`), short lambdas.
- `case` labels sit at the same level as `switch`. A `case` with several statements gets a 4-space-indented body.
- Always use braces for multi-line `if/for/while` bodies.

**Structure**
- One main class per file, file name = class name (`OpenglTexture.hpp/.cpp`). Headers use `.hpp` and `#pragma once`.
- Everything in `namespace CoreEngine`. Don't create sub-namespaces, and don't put `using namespace` inside the engine. Put types shared between modules in the most fitting header, not in two places (e.g. `InputCodes.hpp`, `EventContext.hpp`). When names collide, rename or merge the type instead of separating them by namespace.
- Backend implementations go in a subfolder named after the backend (`Renderer/Opengl/`, `Window/Wx/`, `Input/Wx/`); interfaces and shared types sit at the module root. Objects are created through a `*Factory` keyed by a `*Spec` enum.
- Use `enum class`. Mark types exposed to clients with `CORE_API`. Use `Unique<T>`/`Shared<T>` for ownership; a raw pointer means borrowed, unless the API explicitly takes ownership (e.g. `InjectedWindow`).
- Keep wxWidgets/GLFW/GL headers in `.cpp` files or backend headers; forward-declare in shared headers (see `IWindowVisitor.hpp`).
