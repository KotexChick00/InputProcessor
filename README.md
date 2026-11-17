# InputProcessor — viewer mô hình 3D

## Hướng dẫn cho người mới

Nếu đây là lần đầu bạn mở dự án C++, hãy đọc theo thứ tự:

1. [Cài công cụ và chạy chương trình từng bước](docs/01-HUONG-DAN-CAI-DAT-VA-CHAY.md) — tải repo, cài thư viện, dùng VS Code/Visual Studio hoặc Ubuntu, mở model và xử lý lỗi thường gặp.
2. [Hiểu cấu trúc và luồng mã nguồn](docs/02-GIAI-THICH-MA-NGUON.md) — biết file nào khởi chạy, model đi qua những bước nào, và camera/GPU tham gia ra sao.

Chương trình chính là target `DomainDemo` (`main.cpp`): viewer mô hình 3D chạy trên ECS, đọc GLTF/GLB/OBJ/FBX qua Assimp. Chọn target `DomainDemo` để mở mô hình của bạn.

Ứng dụng dùng backend wxWidgets mặc định, qua các interface window/input/application có sẵn. Một vòng lặp của engine dispatch sự kiện wx, cập nhật camera, render, reset input rồi swap buffers. GLFW vẫn có thể được chọn bằng cấu hình, nhưng không được khởi động khi dùng wx.

Đọc `.gltf` và `.glb` bằng Assimp: mesh, transform của node, normal, UV, vertex color, màu base color và texture ngoài hoặc texture PNG/JPEG nhúng. Viewer hiển thị mô hình tĩnh với ánh sáng đơn giản; animation, skinning và pipeline PBR đầy đủ nằm ngoài phạm vi nhiệm vụ trong ảnh.

Phần camera dùng nguyên namespace `Domain::Camera::OrbitCamera`, `InputMap` và state machine của nhánh `feat/domain` (`35c6905`). Các TODO bàn phím được hoàn thành trong `States/OrbitingState.cpp`, `PanningState.cpp` và `DollyingState.cpp`; không còn controller riêng ở `Domain/Camera/`.

`Application::GetInput()` trả về tham chiếu tới input được lưu trong application, để controller không giữ tham chiếu tới một giá trị tạm. Resource manager dùng tên header chuẩn `IResourceManager.hpp` và giữ cả factory framebuffer của nhóm lẫn API upload texture nhúng cho GLB.

Viewer nằm ở `main.cpp` (target `DomainDemo`). `examples/gltf_viewer.cpp` là viewer cũ trước khi có ECS, không còn được build, giữ lại để tham khảo.

## Kiến trúc scene (ECS)

ECS lõi nằm ở `Core/ECS/` (entity, component, system, không phụ thuộc `Domain`). Lớp scene của viewer nằm ở `Domain/Scene/`:

| Thành phần | Vai trò |
|---|---|
| `SceneLoader` | Đọc file thành entity: mỗi node của file là 1 entity giữ transform cục bộ; node có mesh mang `MeshRenderer`; đèn trong file (glTF `KHR_lights_punctual`) gắn vào node của nó; file không có đèn thì tạo "Default Light". File quá 4000 node được gộp thành 1 entity. |
| Component | `Name`, `Transform`, `Hierarchy`, `MeshRenderer`, `Bounds`, `ModelSource`, `Light`, `Camera`, `RenderSettings` — chỉ chứa dữ liệu. |
| `TransformSystem` → `CameraSystem` → `LightingSystem` → `RenderSystem` | Tính ma trận world theo cây; điều khiển camera orbit; gom tối đa 8 đèn; vẽ entity đang hiển thị theo wireframe/texture/lighting. Thứ tự chạy khai báo trong `AddViewerSystems()`. |
| `ViewState` | Lưu/nạp **trạng thái xem** dạng JSON: đường dẫn model, camera, tuỳ chọn hiển thị, node bị ẩn, đèn bị tắt. Không lưu hình học, không ghi vào file model. |
| `ViewerController` | API cho tầng ứng dụng/GUI: `OpenModel`, `SaveView`/`LoadView`, `GetEntityTree`, `GetModelStats`, `SetVisible`, `SetLightEnabled`, `GetRenderSettings`, `FitCamera`. GUI trong `Core` không gọi trực tiếp (Core không phụ thuộc Domain) mà nối qua `main.cpp`. |

Hình vẽ qua ECS được kiểm tra giống từng pixel với viewer cũ (`tests/test_scene_gl.cpp`).

## Build dùng chung cho VS Code và Visual Studio

Dùng `CMakePresets.json` ở thư mục gốc làm cấu hình build chung. Không cần sửa đường dẫn thư viện trong IDE. CMake chọn generator theo máy; build preset chọn đúng Debug/Release cả khi generator có nhiều cấu hình. CMake cần phiên bản 3.21 trở lên và compiler C++20 hỗ trợ `std::format` (GCC 13+, hoặc MSVC của Visual Studio 2022 trở lên).

### Windows: chuẩn bị một lần

Cài workload **Desktop development with C++** và thành phần CMake trong Visual Studio hoặc Visual Studio Build Tools. Người dùng VS Code chỉ cần Build Tools để có cùng compiler MSVC với đồng đội dùng Visual Studio.

Nếu chưa có vcpkg, chạy trong PowerShell:

```powershell
git clone https://github.com/microsoft/vcpkg C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', $env:VCPKG_ROOT, 'User')
```

Nếu đã có vcpkg, đặt `VCPKG_ROOT` bằng đường dẫn thư mục đó. Mở lại IDE sau khi đặt biến môi trường. CMake sẽ cài các thư viện theo `vcpkg.json` và baseline đã khóa trong `vcpkg-configuration.json` khi configure lần đầu; cần internet và có thể mất thời gian.

### VS Code

1. Mở thư mục gốc chứa `CMakeLists.txt`; cài hai extension được đề xuất: **C/C++** và **CMake Tools**.
2. Chạy **CMake: Select Configure Preset**, chọn `vcpkg-debug` trên Windows hoặc `debug` khi đã có thư viện hệ thống.
3. Chạy **CMake: Configure**, rồi **CMake: Build**.
4. Chạy **CMake: Set Launch/Debug Target**, chọn `DomainDemo`; chạy **CMake: Debug** hoặc **CMake: Run Without Debugging**.

`.vscode/settings.json` đã cấu hình IntelliSense dùng CMake và debugger mở mô hình mẫu. Đổi `cmake.debugConfig.args` nếu muốn debug với file của bạn. Run Without Debugging cũng mở mô hình mẫu nếu không truyền đối số.

### Visual Studio

Mở **File → Open → Folder** và chọn thư mục dự án, rồi chọn preset `vcpkg-debug`. Sau khi configure xong, chọn target `DomainDemo` làm startup item, build và chạy bằng F5/Ctrl+F5. Khi không truyền đối số, viewer tự mở mô hình mẫu.

Đây là đường build dùng chung với VS Code. Project `.slnx` riêng vẫn được giữ cho ai dùng cấu hình project truyền thống; project đó chỉ chạy với Visual Studio đã cài đúng toolset `v145`. Nếu không có toolset này, mở thư mục bằng CMake như trên để dùng compiler hiện có.

### Terminal

Trong Developer PowerShell / x64 Native Tools Command Prompt trên Windows:

```sh
cmake --preset vcpkg-debug
cmake --build --preset vcpkg-debug
ctest --preset vcpkg-debug
```

Thay `vcpkg-debug` bằng `vcpkg-release` để build Release. Các preset `debug`/`release` dùng thư viện hệ thống; có thể ghi tùy chỉnh riêng của máy trong `CMakeUserPresets.json` (đã được gitignore).

Trên Ubuntu 24.04, cài dependency rồi dùng preset hệ thống:

```sh
sudo apt install cmake g++ libwxgtk3.2-dev libassimp-dev libglfw3-dev libglm-dev libspdlog-dev libstb-dev libgl-dev
cmake --preset debug
cmake --build --preset debug
```

GLAD dùng source có sẵn trong `Library/Glad`. CMake ưu tiên các target wxWidgets từ vcpkg để liên kết đúng Debug/Release; thư viện hệ thống được tìm qua `wx-config`.

## Chạy

```sh
./build/debug/DomainDemo tests/assets/textured.gltf
./build/debug/DomainDemo tests/assets/lights.gltf
./build/debug/DomainDemo --view tests/assets/lights.view.json
```

Trên Windows với generator Visual Studio, executable Debug nằm trong `build/vcpkg-debug/Debug/DomainDemo.exe`; generator Ninja đặt executable trực tiếp trong `build/vcpkg-debug/`. Chạy không có đối số sẽ mở mẫu `textured.gltf`. Tùy chọn `--frames N` đóng viewer sau N frame để kiểm tra tự động. Tùy chọn `--view FILE.json` chọn file trạng thái xem: nếu file đã có, viewer mở theo nó (model + camera + tùy chọn); F5 lưu vào file này (mặc định `<tên model>.view.json` trong thư mục hiện tại), F9 nạp lại.

| Thao tác | Điều khiển |
|---|---|
| Xoay camera | Giữ chuột trái và kéo, hoặc phím mũi tên |
| Zoom | Cuộn chuột |
| Dịch chuyển điểm nhìn | Giữ chuột phải và kéo, hoặc W / A / S / D |
| Dolly (thay đổi khoảng cách) | Giữ chuột giữa và kéo, hoặc PageUp / PageDown |
| Căn lại camera theo mô hình | R |
| Bật/tắt wireframe / texture / ánh sáng | 1 / 2 / 3 |
| Lưu / nạp trạng thái xem | F5 / F9 |
| Đóng ứng dụng | Escape hoặc nút đóng cửa sổ |

Các phím và nút được bind qua `InputMap`, có thể đổi bằng `GetInputMap().BindAction(...)`. State machine giữ quy tắc mỗi thời điểm một gesture dùng mouse delta; gesture vừa nhấn có thể giành quyền, gesture còn giữ được tiếp tục sau khi gesture hiện tại kết thúc. Bàn phím dùng delta time; di chuột không giữ nút không được tính là kéo khi đang điều khiển bằng phím.

Camera được căn theo bounding box của vertex đã áp dụng transform node; giới hạn zoom và clipping thay đổi theo kích thước mô hình. Resize cập nhật viewport và aspect ratio; mất focus/capture xóa các phím/nút đang giữ.

## Kiểm thử

```sh
ctest --preset debug
```

Các test GUI cần display và OpenGL 3.3. Trên Linux CI không có display:

```sh
GDK_BACKEND=x11 xvfb-run -a ctest --preset debug
```

Bộ kiểm thử bao gồm:

- Camera/state: xoay, pan, dolly bằng bàn phím, rebind, chuyển gesture, thả phím, reset sau fit, giới hạn zoom, projection và viewport bằng 0.
- Input: pressed/held/released/just-pressed và reset delta/scroll theo frame.
- Đọc GLTF thật: data URI, buffer/texture ngoài, GLB với PNG nhúng, transform node, file lỗi/không tồn tại, đuôi viết hoa và handler không có fallback, texture dùng chung và import lặp lại.
- Tích hợp wx: event input qua cửa sổ thật, resize, mất focus, upload texture nhúng, yêu cầu đóng qua API, nút đóng cửa sổ và mở lại ứng dụng; input ổn định cho controller và framebuffer màu của nhóm.
- Scene/ECS (`test_scene`, không cần OpenGL): cây entity, transform nhiều cấp, node có shear, đọc đèn, so cây node với bản gộp trên 6 file, từng system (renderer giả), lưu/nạp trạng thái xem (JSON hỏng, sai version, node không còn), `ViewerController`.
- Pipeline ECS với OpenGL thật (`test_scene_gl`): shader biên dịch, ảnh giống từng pixel với viewer cũ, wireframe/texture/lighting đổi hình.
- Chạy viewer ba frame bằng `--frames 3` (GLTF, GLB, đèn, mẫu mặc định, `--view`), file thiếu và file trạng thái hỏng phải báo lỗi.

Các fixture nhỏ trong `tests/assets` được tạo cho dự án, không phụ thuộc tải model bên ngoài.

Đã kiểm tra thực thi trên Linux; chạy build/test trên Windows vẫn cần thực hiện trên máy Windows. Các cấu hình IDE không chứa đường dẫn riêng của máy phát triển.
