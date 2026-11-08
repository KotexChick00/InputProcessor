# CHANGES.md — Critical Fixes for InputProcessor

Bản sửa lỗi Critical (C1–C18) từ code review. Mỗi mục có `file:line`, mô tả, lý do, và cách sửa.

## Giới hạn verify

Sandbox Linux không có root để cài các thư viện hệ thống (wxWidgets, Assimp, GLFW, spdlog, stb, glm), và build vcpkg từ source vượt quá thời gian cho phép. Do đó, verify được thực hiện bằng **syntax-only check** (`g++ -std=c++20 -fsyntax-only`) với **stub headers** cho 6 thư viện còn thiếu. Điều này bắt được:

- Lỗi cú pháp do fix gây ra
- Lỗi type mismatch
- Lỗi include/scope
- Lỗi khởi tạo member

Không verify được:

- Link errors
- Runtime behavior
- Tương thích GL/wx API thật

Khi build bằng toolchain thật (vcpkg hoặc apt install deps), vẫn cần chạy `cmake --build` + `ctest` để verify đầy đủ.

## Tóm tắt 18 fix Critical

### C1 — `Unique<T>` là `shared_ptr` (Critical)
**File:** `Core/pch.h:24`
**Trước:**
```cpp
template<typename T> using Unique = std::shared_ptr<T>;
```
**Sau:**
```cpp
template<typename T> using Unique = std::unique_ptr<T>;
```
**Lý do:** Mọi `Unique<>` trong engine (Application, GLFWWindow, WxWindow, Logger) tưởng là độc quyền nhưng thực ra chia sẻ → che giấu double-free/use-after-free.
**Tác động phụ:** `Core/Window/Wx/WxWindow.cpp:310-311` đổi `std::make_shared` → `std::make_unique` cho `mKeyboardInput`/`mMouseInput` để khớp unique_ptr.

### C2 — `Time::mDeltaTime`/`mTimeNow` không khởi tạo (Critical, UB)
**File:** `Core/Time/Time.hpp:27`
**Trước:** `float mDeltaTime, mTimeScale = 1.0f, mTimeNow;`
**Sau:** `float mDeltaTime{0.0f}, mTimeScale{1.0f}, mTimeNow{0.0f};`
**Lý do:** `gltf_viewer.cpp:75` và `main.cpp:159` gọi `GetDeltaTime()` ở frame đầu, đọc giá trị không xác định → camera/delta-time movement có thể nhảy một bước khổng lồ.

### C3 — `Time::Update()` khai báo nhưng không định nghĩa (Critical, linker error)
**File:** `Core/Time/Time.hpp:18`
**Trước:** `void Update();` (khai báo, không có định nghĩa → linker error nếu gọi)
**Sau:** `void Update() {}` (no-op, giữ interface)
**Lý do:** Bất kỳ caller nào gọi `Time::Update()` sẽ linker error. Hiện tại chưa ai gọi nhưng là bug tiềm ẩn.

### C4 — `static` cursor delta trong GLFW callback (Critical, UB cross-window)
**File:** `Core/Window/GLFW/GLFWWindow.cpp:51-80` + `GLFWWindow.hpp:52-57`
**Trước:**
```cpp
glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xPos, double yPos) {
    ...
    static double lastFrameX = xPos;
    static double lastFrameY = yPos;
    double dx = xPos - lastFrameX;
```
**Sau:** Chuyển `lastFrameX/Y` thành member `mLastCursorX/Y` + `mHaveLastCursorPos` của `GLFWWindow`. Callback đọc/ghi qua `handler->mLastCursorX` etc.
**Lý do:** `static` chia sẻ state giữa mọi `GLFWWindow` instance và giữa các create/destroy cycle → delta sai. Hai cửa sổ → dx tính bằng vị trí chuột của cửa sổ khác.

### C5 — `GLFWWindow::Close()` gọi `glfwTerminate()` (Critical, state corruption)
**File:** `Core/Window/GLFW/GLFWWindow.cpp:155-168`
**Trước:**
```cpp
void GLFWWindow::Close() {
    if (mWindow) {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
        glfwTerminate();   // phá hủy toàn bộ GLFW process-wide
        IP_ENGINE_TRACE("Window closed and GLFW terminated.");
    }
}
```
**Sau:** Bỏ `glfwTerminate()`, chỉ `glfwDestroyWindow`. Thêm comment giải thích process-wide teardown nên ở scope application.
**Lý do:** `glfwTerminate()` phá hủy mọi GLFWwindow và resource trong process, kể cả `imgui_impl_glfw` backend và các `GLFWWindow` khác.

### C6 — Downcast C-style trong `EventAction` (Critical, UB)
**File:** `Core/Event/EventAction.hpp:20`
**Trước:** `return mCallback(*(T*)&eventContext);`
**Sau:** `return mCallback(static_cast<const T&>(eventContext));`
**Lý do:** C-cast `(T*)&eventContext` cast `const EventContext*` → `T*` (bỏ const), deref → UB nếu eventContext thực sự const. Dispatcher đã route theo `typeid` nên `static_cast<const T&>` an toàn.

### C7 — `~WxWindow()` double-free khi force-close (Critical)
**File:** `Core/Window/Wx/WxWindow.cpp:281-304`
**Trước:**
```cpp
WxWindow::~WxWindow() {
    if (mGLCanvas && mGLCanvas->HasCapture()) mGLCanvas->ReleaseMouse();
    delete mGLContext;
    delete mFrame;
    ...
}
```
**Sau:** Null-check + `mFrame->Destroy()` (thay `delete`) + null-out tất cả member sau khi release.
**Lý do:** Khi OS ép đóng (taskkill, logout), wxWidgets tự hủy frame → `delete mFrame` double-free. `Destroy()` an toàn hơn vì wx quản lý child destruction.

### C8 — `static_cast` xuống `Wx*` không RTTI check (Critical, UB)
**File:** `Core/Window/Wx/WxWindow.cpp:124,142,188,208,235-236` (5 chỗ)
**Trước:**
```cpp
static_cast<Input::Wx::WxMouseInput*>(mouseInput)->NotifyButtonDown(inputBtn);
```
**Sau:**
```cpp
auto* wxMouse = dynamic_cast<Input::Wx::WxMouseInput*>(mouseInput);
CORE_ASSERT(wxMouse != nullptr && "WxEngineCanvas expects a WxMouseInput");
if (wxMouse) wxMouse->NotifyButtonDown(inputBtn);
```
**Lý do:** Engine hỗ trợ injection (`InjectedWindow`). Nếu inject một `MouseInput` loại khác → `static_cast` UB ngay. `dynamic_cast` + assert bắt được rõ ràng.

### C9 — UI interfaces thiếu virtual destructor (Critical, UB)
**File:** `Core/UI/IUIRenderer.hpp`, `Core/UI/Elements/IUIElement.hpp`, `Core/UI/Elements/IUIElementFactory.hpp`
**Sửa:** Thêm `virtual ~ClassName() = default;` cho:
- `IUIRenderer`
- `IResizeableUIElement`
- `IUIElement`
- `IPositionSetableUIElement`
- `IUIElementFactory`
**Lý do:** `delete` qua base pointer không có virtual dtor → UB.
**Bonus:** `IContainer`/`IButton` private inheritance `IResizeableUIElement`/`IPositionSetableUIElement` → đổi sang `public` để `c->Resize(...)` compile được.

### C10 — `~ImguiUIRenderer()` khai báo nhưng không định nghĩa (Critical, linker error)
**File:** `Core/UI/Imgui/ImguiUIRenderer.hpp` + `ImguiUIRenderer.cpp`
**Trước:** Header khai báo `~ImguiUIRenderer();`, không có định nghĩa trong cpp.
**Sau:**
- Đổi raw owning pointer `ImguiWindowContextInitVisitor* mContextVisitor = nullptr;` etc. → `std::unique_ptr<...>`
- Định nghĩa `~ImguiUIRenderer()` trong cpp, gọi `mShutdownVisitor->Visit` nếu Init đã được gọi, rồi unique_ptr destructors dọn dẹp
- `Init()` thêm guard chống double-call
- `Render()` thêm null-check visitor
- `Free()` null-out all member sau reset
**Lý do:** Bất kỳ TU nào construct/destroy `ImguiUIRenderer` → linker error. unique_ptr khắc phục leak nếu Init quên hoặc Free gọi 2 lần.

### C11 — `RenderResourceFactory::CreateTexture` không compile được (Critical)
**File:** `Domain/Model/RenderResourceFactory.hpp` + `RenderResourceFactory.cpp`
**Trước:**
```cpp
auto texture = std::make_unique<ITexture>();   // ITexture có pure virtual!
```
(Không đọc `filePath`, không load texture)
**Sau:** `RenderResourceFactory` nhận `IResourceManager*` qua constructor, `CreateTexture(filePath)` delegate sang `mResourceManager->CreateTexture(filePath)` với path-keyed cache.
**Lý do:** `make_unique<ITexture>()` trên abstract class không compile. File không được CMake glob (chính vì vậy) nên build pass, nhưng code chết.

### C12 — `ObjModelFormatHandler::Handle` không có impl (Critical, linker error)
**File:** `Domain/Model/Importers/ModelImporter/ObjModelFormatHandler.hpp`
**Trước:** Header khai báo `RenderModel* Handle(const char* file) override;` không có .cpp.
**Sau:** Inline implementation delegate sang `Next(file)` cho extension `.obj`. Thêm `<algorithm>`, `<cctype>`, `<filesystem>`, `<string>` includes.
**Lý do:** Bất kỳ TU nào instantiate `ObjModelFormatHandler` → linker error.

### C13 — `FreeCameara.hpp` rỗng, sai chính tả (Critical, dead code)
**File:** `Domain/Camera/FreeCamera/FreeCameara.hpp`
**Hành động:** Xóa file và thư mục `FreeCamera/`.
**Lý do:** File chỉ có `#pragma once`, không có impl. Filename sai (`Cameara`). Vi phạm "no speculative abstractions" (CLAUDE.md).

### C14 — `main.cpp` path `Ak-47.obj` không tồn tại (Critical, silent blank window)
**File:** `main.cpp:126`
**Trước:** `const std::string modelPath = "./assets/models/Ak_47/Ak-47.obj";`
**Sau:** `const std::string modelPath = "./assets/models/Cube/Cube.fbx";`
**Lý do:** Repo chỉ có `Ak-47.mtl` + JPG, không có `.obj`. `AssimpBoundingBox::FromFile` trả box không hợp lệ, `Import` trả null, render bị skip, người dùng thấy cửa sổ đen không diagnostic. `Cube.fbx` có sẵn trong repo.

### C15 — `.vcxproj` toolset `v145` không tồn tại (Critical, VS refuse load)
**File:** `ObjFileInutProcessor.vcxproj:33,39,46,52`
**Trước:** `<PlatformToolset>v145</PlatformToolset>` (4 chỗ)
**Sau:** `<PlatformToolset>v143</PlatformToolset>` (VS 2022)
**Lý do:** `v143` = VS 2022, `v144` = VS 2026 Preview. Không có `v145` shipping. VS refuse load hoặc fallback âm thầm.

### C16 — `.vcxproj` compile sai entry point (Critical, VS build ra demo hỏng)
**File:** `ObjFileInutProcessor.vcxproj:217`
**Trước:** `<ClCompile Include="main.cpp" />` (DomainDemo, đã hỏng theo C14)
**Sau:** `<ClCompile Include="examples\gltf_viewer.cpp" />` (viewer thật theo CMake)
**Lý do:** Project tên `InputProcessor` nhưng compile `main.cpp` → VS user build ra DomainDemo hỏng, không phải viewer.

### C17 — Test dùng `assert()` thuần, có thể no-op trong Release (Critical)
**File:** `tests/test_engine_logic.cpp:1-7`, `tests/test_wx_integration.cpp:1-7`
**Trước:** `#include <cassert>` + 87+36 lệnh `assert(...)`.
**Sau:** Thêm ở đầu mỗi file:
```cpp
#ifdef NDEBUG
#undef NDEBUG
#endif
```
**Lý do:** CMake cố `/UNDEBUG`/`-UNDEBUG` (CMakeLists.txt:63-67) nhưng fragile với `clang-cl` và phụ thuộc thứ tự flag. Nếu `NDEBUG` được define, toàn bộ test "pass" ảo vì assert là no-op. `#undef NDEBUG` đầu file đảm bảo assert luôn active.

### C18 — `FormatChecker` trả `Ascii_OBJ` khi file không tồn tại (Critical, silent mis-route)
**File:** `src/FormatChecker.cpp` (toàn bộ)
**Sửa:**
1. `if (!file.is_open()) return FileFormat::Unknown;` ngay sau khi mở
2. `solid` magic không kết luận `Ascii_STL` ngay — set flag, defer đến sau binary size check
3. `tellg()` fail (`pos == std::streampos(-1)`) → return `Unknown`
4. Đọc 4 byte `numTriangles` explicit little-endian (4 byte vào `unsigned char[4]`, assemble) thay vì `reinterpret_cast<uint32_t*>`
5. Binary STL check `>=` thay vì `==` để chấp nhận file có trailing metadata
6. `if (startsWithSolid) return Ascii_STL;` chỉ đạt nếu binary size check fail
**Lý do:**
- File thiếu → `bytesRead == 0` → mọi magic check skip → return `Ascii_OBJ` (sai, caller cố parse OBJ file không có)
- Binary STL có "solid" trong 80-byte header (Fusion360, SolidWorks) → misclassify
- `tellg()` fail trả `pos_type(-1)` cast sang `size_t` = ~0ULL → nonsense
- `reinterpret_cast<uint32_t*>` giả định little-endian, UB strict aliasing
- `==` exact size reject STL có trailing metadata (false negative)

## Bonus cleanup trong vcxproj/filters

Trong quá trình sửa C15/C16, cũng làm sạch:
- **C15a:** Bỏ stale references: `src\OBJ\ObjParser.h`, `UI\ImGuiUIRendererAdapter.hpp`, `Core\Importer\IImporter.hpp`, `Core\Renderer\Resource\IPostProcessEffect.hpp`, `Core\Renderer\Resource\IPostProcessStack.hpp`
- **C15b:** Thêm missing references: `examples\gltf_viewer.cpp`, `Domain\Model\RenderResourceFactory.cpp`, `Domain\Model\RenderResourceFactory.hpp`, `Domain\Model\Importers\ModelImporter\GltfModelFormatHandler.hpp`
- **C15c:** Bỏ reference `Domain\Camera\FreeCamera\FreeCameara.hpp` (file đã xóa theo C13)
- **C15d:** Dedupe `Core\Event\EventDispatcher.hpp` (bị list 2 lần)
- **C15e:** Thêm include `<cfloat>` cho `FLT_MAX` trong `IUIElement.hpp` (lỗi compile nếu include riêng lẻ)
- **C15f:** Sửa `__gcc__` → `__GNUC__` và `__gcc__` → `__GNUC__` trong `pch.h:52,59` (GCC define `__GNUC__`, không phải `__gcc__`)
- **C15g:** Đổi `<stdint.h>` → `<cstdint>` trong `pch.h:11` (không pollute global namespace)

## Bonus fix phát sinh trong khi verify

### B1 — `WxEngineFrame` ctor truyền `wxDefaultSize` không khớp constructor
**File:** `Core/Window/Wx/WxWindow.cpp:27`
**Sửa:** Đổi `: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size)` → `: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)` và `(void)size;` để tránh unused warning. Size thực sự áp qua `SetClientSize` ngay sau.

### B2 — `WxWindow::Init` truyền `wxDefaultSize` thay vì size thực
**File:** `Core/Window/Wx/WxWindow.cpp:347`
**Sửa:** `new WxEngineFrame(this, ..., wxDefaultSize)` → `new WxEngineFrame(this, ..., wxSize(config.Width, config.Height))`

## Files đã sửa (10 source files + 2 vcxproj)

| File | Fix |
|---|---|
| `Core/pch.h` | C1, C15f, C15g |
| `Core/Time/Time.hpp` | C2, C3 |
| `Core/Event/EventAction.hpp` | C6 |
| `Core/Window/GLFW/GLFWWindow.hpp` | C4 |
| `Core/Window/GLFW/GLFWWindow.cpp` | C4, C5 |
| `Core/Window/Wx/WxWindow.cpp` | C7, C8, B1, B2 |
| `Core/UI/IUIRenderer.hpp` | C9 |
| `Core/UI/Elements/IUIElement.hpp` | C9, C15e |
| `Core/UI/Elements/IUIElementFactory.hpp` | C9 |
| `Core/UI/Imgui/ImguiUIRenderer.hpp` | C10 |
| `Core/UI/Imgui/ImguiUIRenderer.cpp` | C10 |
| `Domain/Model/RenderResourceFactory.hpp` | C11 |
| `Domain/Model/RenderResourceFactory.cpp` | C11 |
| `Domain/Model/Importers/ModelImporter/ObjModelFormatHandler.hpp` | C12 |
| `main.cpp` | C14 |
| `src/FormatChecker.cpp` | C18 |
| `tests/test_engine_logic.cpp` | C17 |
| `tests/test_wx_integration.cpp` | C17 |
| `ObjFileInutProcessor.vcxproj` | C15, C16, C15a–e |
| `ObjFileInutProcessor.vcxproj.filters` | C15a–e |
| `Domain/Camera/FreeCamera/FreeCameara.hpp` | C13 (xóa) |

## Verify kết quả

Syntax-only check với stub headers (10 file đã sửa):
- `examples/gltf_viewer.cpp` — PASS
- `main.cpp` — PASS
- `tests/test_engine_logic.cpp` — PASS
- `tests/test_wx_integration.cpp` — PASS
- `Core/Window/GLFW/GLFWWindow.cpp` — PASS
- `Core/Window/Wx/WxWindow.cpp` — PASS
- `src/FormatChecker.cpp` — PASS
- `src/AsciiReader.cpp` — PASS
- `Domain/Model/RenderResourceFactory.cpp` — PASS
- `Core/UI/Imgui/ImguiUIRenderer.cpp` — PASS

## Bước tiếp theo cho bạn

1. **Build bằng toolchain thật** (vcpkg trên Windows, hoặc `apt install libwxgtk3.2-dev libassimp-dev libglfw3-dev libglm-dev libspdlog-dev libstb-dev libgl-dev` trên Ubuntu 24.04):
   ```sh
   cmake --preset debug
   cmake --build --preset debug
   ctest --preset debug
   ```

2. **Nếu có lỗi build thật**, báo lại cho tôi — đặc biệt là các fix đụng đến ownership (`C1`), callback lambda capture (`C4`), và wxWidgets API (`C7`, `B1`, `B2`).

3. **Sau khi build pass**, có thể tiếp tục với High-priority fixes (H1–H65) từ báo cáo review gốc.
