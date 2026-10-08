# Hướng dẫn cài đặt và chạy InputProcessor

Tài liệu này dành cho người lần đầu mở dự án C++ bằng CMake. Làm lần lượt các bước trong phần ứng với máy của bạn: Windows với VS Code, Windows với Visual Studio hoặc Ubuntu.

## 1. Chương trình làm gì?

InputProcessor mở mô hình 3D dạng GLTF hoặc GLB, đọc hình học và vật liệu, rồi hiển thị mô hình trong cửa sổ. Bạn có thể xoay, zoom và di chuyển camera.

Repo có hai chương trình:

| Tên chương trình | Chức năng | Điều kiện |
|---|---|---|
| InputProcessor | Viewer chính cho GLTF và GLB. | Có sẵn mô hình mẫu trong repo; có thể mở thêm mô hình của bạn. |
| DomainDemo | Demo OBJ cũ của nhóm. | Cần file assets/models/Ak_47/Ak-47.obj cùng các file vật liệu và texture mà OBJ tham chiếu. File này không có trong repo hiện tại. |

Hãy chạy InputProcessor nếu muốn xem mô hình của mình. DomainDemo vẫn được build, nhưng nếu thiếu OBJ nói trên thì cửa sổ sẽ không có mô hình.

## 2. Tải mã nguồn về máy

Repo đang ở chế độ Private. Đăng nhập GitHub bằng tài khoản có quyền xem repo trước khi tải. Cách dễ nhất là dùng HTTPS:

1. Mở [repo InputProcessor-GLTF](https://github.com/vietcuong13/InputProcessor-GLTF).
2. Nhấn Code, chọn HTTPS, rồi sao chép địa chỉ.
3. Mở Terminal, PowerShell hoặc Developer PowerShell.
4. Tạo rồi mở thư mục lưu dự án. Ví dụ trên Windows:

~~~powershell
New-Item -ItemType Directory -Force C:\dev | Out-Null
Set-Location C:\dev
~~~

5. Clone dự án. Nếu địa chỉ ở nút Code khác với ví dụ, thay bằng địa chỉ bạn vừa sao chép:

~~~powershell
git clone https://github.com/vietcuong13/InputProcessor-GLTF.git
cd InputProcessor-GLTF
~~~

Git có thể mở trình duyệt để đăng nhập. Không dán mật khẩu hoặc token vào lệnh clone. Nếu đã cấu hình SSH, bạn cũng có thể sao chép địa chỉ SSH từ nút Code và dùng lệnh git clone với địa chỉ đó.

Terminal cần đang ở thư mục có CMakeLists.txt. Kiểm tra bằng:

~~~powershell
dir CMakeLists.txt
~~~

Trên Ubuntu dùng ls CMakeLists.txt.

Trên Ubuntu, dùng Terminal và chạy:

~~~bash
mkdir -p ~/dev
cd ~/dev
git clone https://github.com/vietcuong13/InputProcessor-GLTF.git
cd InputProcessor-GLTF
~~~

## 3. Cài công cụ và thư viện

### Windows: VS Code hoặc Visual Studio

Cả hai IDE dùng cùng CMake Presets. Cài các công cụ sau:

1. Git for Windows để tải mã nguồn.
2. Visual Studio 2022 hoặc 2026 (hoặc Build Tools cùng phiên bản), kèm workload Desktop development with C++.
3. Trong workload đó, giữ MSVC C++ build tools và C++ CMake tools.
4. CMake phiên bản 3.21 trở lên. Visual Studio đã kèm sẵn CMake; nếu dùng Terminal, hãy dùng Developer PowerShell for VS (xem mục 6).
5. vcpkg để CMake tự tải thư viện dự án cần.

Trên Windows, dự án chỉ build bằng MSVC. Preset vcpkg-debug và vcpkg-release luôn chọn cl.exe, nên MinGW, Strawberry Perl hay Anaconda có gcc trong PATH cũng không ảnh hưởng. Nếu CMake vẫn nhận nhầm compiler khác, nó sẽ dừng ngay khi configure và in hướng dẫn sửa.

Nếu vcpkg chưa được cài, cài một lần bằng PowerShell. Nếu bạn đã có thư mục C:\dev\vcpkg, bỏ qua hai lệnh clone/bootstrap và dùng các lệnh đặt biến môi trường bên dưới:

~~~powershell
New-Item -ItemType Directory -Force C:\dev | Out-Null
Set-Location C:\dev
git clone https://github.com/microsoft/vcpkg C:\dev\vcpkg
& C:\dev\vcpkg\bootstrap-vcpkg.bat
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\dev\vcpkg', 'User')
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
~~~

Nếu vcpkg đã nằm ở thư mục khác, thay C:\dev\vcpkg bằng đúng đường dẫn đó. Đóng rồi mở lại VS Code hoặc Visual Studio để IDE nhận biến môi trường. Kiểm tra:

~~~powershell
$env:VCPKG_ROOT
Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
~~~

Lệnh thứ hai cần in True. Lần đầu cấu hình, vcpkg tải và biên dịch các thư viện trong vcpkg.json. Việc này cần Internet và có thể mất vài phút; hãy chờ CMake hoàn tất.

Các thư viện gồm wxWidgets để tạo cửa sổ, Assimp để đọc mô hình, OpenGL để vẽ, cùng GLFW, GLM, spdlog và stb. Phiên bản dependency vcpkg được khóa trong vcpkg-configuration.json để các máy dùng cùng mốc thư viện.

### Ubuntu 24.04

Mở Terminal rồi cài compiler và thư viện hệ thống:

~~~bash
sudo apt update
sudo apt install cmake g++ libwxgtk3.2-dev libassimp-dev libglfw3-dev libglm-dev libspdlog-dev libstb-dev libgl-dev
~~~

Kiểm tra compiler và CMake:

~~~bash
cmake --version
g++ --version
~~~

Cần CMake 3.21 trở lên và compiler có C++20 cùng std::format. Ubuntu 24.04 kèm GCC 13 có thể dùng preset debug tìm thư viện hệ thống.

## 4. Chạy trên Windows bằng VS Code

1. Mở VS Code.
2. Chọn File → Open Folder... và mở thư mục InputProcessor-GLTF vừa clone. Hãy mở thư mục có CMakeLists.txt, không mở riêng thư mục Core hoặc examples.
3. Nếu VS Code hỏi có tin cậy thư mục hay không, hãy chọn tin cậy khi bạn đã clone đúng repo.
4. Cài hai extension được đề xuất: C/C++ và CMake Tools.
5. Nhấn Ctrl+Shift+P để mở Command Palette.
6. Chạy CMake: Select Configure Preset và chọn vcpkg-debug.
7. Chạy CMake: Configure. Lần đầu có thể tải thư viện bằng vcpkg; đợi tới khi CMake báo cấu hình xong.
8. Chạy CMake: Set Build Target và chọn InputProcessor.
9. Chạy CMake: Build. Đợi thông báo build thành công.
10. Nếu VS Code hỏi chương trình nào cần chạy, dùng CMake: Set Launch/Debug Target và chọn InputProcessor.
11. Nhấn F5 để chạy với debugger, hoặc chạy CMake: Run Without Debugging để mở cửa sổ bình thường.

Không cần truyền đối số để xem mô hình mẫu. Cấu hình debug trong .vscode/settings.json cũng truyền sẵn đường dẫn tới tests/assets/textured.gltf. Mục 7 giải thích cách truyền mô hình khác.

## 5. Chạy trên Windows bằng Visual Studio

Mở thư mục dự án bằng CMake để hai IDE dùng cùng target và preset. Không cần mở file project riêng.

1. Mở Visual Studio 2022 hoặc 2026.
2. Chọn File → Open → Folder... và mở thư mục InputProcessor-GLTF đã clone.
3. Nếu không thấy preset, mở Tools → Options → CMake → General và bật tích hợp CMakePresets.json. Tùy phiên bản, tùy chọn có thể tên là Always use CMakePresets.json. Sau đó chọn File → Close Folder và mở lại thư mục dự án.
4. Trên thanh công cụ CMake, chọn Configure Preset vcpkg-debug. Nếu được hỏi Target System, chọn Local Machine.
5. Đợi Visual Studio cấu hình và vcpkg cài dependency. Lần đầu cần Internet.
6. Chọn Build Preset vcpkg-debug rồi chọn Build → Build All. Hoặc mở CMake Targets View, nhấp phải target InputProcessor và chọn Build để chỉ build viewer.
7. Trong danh sách CMake Targets, chọn InputProcessor làm target chạy.
8. Nhấn F5 để chạy với debugger hoặc Ctrl+F5 để chạy bình thường. Viewer tự mở mô hình mẫu.

Nếu vẫn không thấy preset, kiểm tra VCPKG_ROOT rồi đóng và mở lại thư mục. Tên mục CMake có thể thay đổi theo phiên bản Visual Studio. Xem tài liệu [CMake Presets trong Visual Studio](https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170).

## 6. Chạy trên Ubuntu hoặc từ Terminal

Từ thư mục gốc repo, cấu hình Debug:

~~~bash
cmake --preset debug
~~~

Nếu cấu hình kết thúc thành công, build viewer:

~~~bash
cmake --build --preset debug --target InputProcessor
~~~

Chạy mô hình mẫu:

~~~bash
./build/debug/InputProcessor
~~~

Terminal vẫn mở trong lúc cửa sổ ứng dụng đang chạy. Nhấn Esc hoặc nút đóng cửa sổ để thoát.

Để tạo bản Release, thay debug bằng release trong cả lệnh cấu hình và build. Trên Windows mới cài Visual Studio, dùng preset vcpkg-debug hoặc vcpkg-release; preset debug/release tìm thư viện đã cài trong hệ điều hành.

### Windows từ Terminal

PowerShell hoặc Terminal thường không có sẵn compiler MSVC (cl.exe). Hãy mở **Developer PowerShell for VS** từ Start menu, chuyển tới thư mục gốc repo, rồi chạy:

~~~powershell
cmake --preset vcpkg-debug
cmake --build --preset vcpkg-debug --target InputProcessor
.\build\vcpkg-debug\InputProcessor.exe
~~~

Nếu chạy trong PowerShell thường, CMake sẽ báo không tìm thấy cl.exe. Khi đó hãy chuyển sang Developer PowerShell, đừng sửa preset để dùng gcc.

Trên Ubuntu thay vcpkg-debug bằng debug.

## 7. Mở mô hình của bạn

Chạy InputProcessor từ thư mục gốc dự án và truyền đường dẫn tới GLTF hoặc GLB. Ví dụ PowerShell:

~~~powershell
.\build\vcpkg-debug\InputProcessor.exe "C:\models\robot.glb"
~~~

Preset vcpkg-debug dùng Ninja nên file chạy nằm tại build\vcpkg-debug\InputProcessor.exe. Nếu không thấy, tìm nó:

~~~powershell
Get-ChildItem .\build\vcpkg-debug -Filter InputProcessor.exe -Recurse
~~~

Trên Ubuntu, ví dụ:

~~~bash
./build/debug/InputProcessor /home/ban/models/robot.gltf
~~~

Chạy viewer mẫu và tự đóng sau 3 khung hình:

~~~powershell
.\build\vcpkg-debug\InputProcessor.exe --frames 3
~~~

Chạy một mô hình cụ thể trong 3 khung hình:

~~~powershell
.\build\vcpkg-debug\InputProcessor.exe "C:\models\robot.glb" --frames 3
~~~

Với file GLTF, giữ nguyên các file .bin và texture bên ngoài mà nó tham chiếu tới, cùng cấu trúc thư mục tương đối. Tùy chọn --frames chủ yếu dùng để kiểm tra; khi không có tùy chọn này, cửa sổ chạy tới khi bạn đóng.

## 8. Điều khiển cửa sổ

| Việc cần làm | Điều khiển |
|---|---|
| Xoay camera | Giữ chuột trái và kéo, hoặc dùng các phím mũi tên. |
| Zoom | Cuộn chuột lên để lại gần, xuống để ra xa. |
| Di chuyển điểm nhìn | Giữ chuột phải và kéo, hoặc nhấn W, A, S, D. |
| Thay đổi khoảng cách camera | Giữ chuột giữa và kéo, hoặc nhấn PageUp/PageDown. |
| Căn lại camera theo mô hình | Nhấn R. |
| Đóng ứng dụng | Nhấn Esc hoặc nút đóng cửa sổ. |

Camera tự căn khi mở mô hình. R chỉ căn lại vị trí camera, không nạp lại file.

## 9. Chạy kiểm tra của dự án

Sau khi configure và build, chạy từ thư mục gốc:

~~~powershell
ctest --preset vcpkg-debug
~~~

Trên Ubuntu dùng ctest --preset debug. Năm kiểm tra bao gồm logic camera/input, tích hợp wxWidgets, nạp GLTF, nạp GLB và mở model mẫu. Các kiểm tra giao diện cần môi trường có màn hình và OpenGL.

## 10. Xử lý lỗi thường gặp

| Lỗi | Cách kiểm tra |
|---|---|
| CMake không tìm thấy compiler hoặc chương trình build | Cài workload Desktop development with C++, rồi khởi động lại IDE. |
| Không tìm thấy vcpkg.cmake | Kiểm tra VCPKG_ROOT và chạy Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake". Đóng/mở lại IDE sau khi đặt biến. |
| Không tìm thấy wxWidgets, assimp, glm hoặc thư viện khác | Trên Windows chọn preset vcpkg-debug và chờ cài xong. Trên Ubuntu cài các gói ở mục 3 rồi chọn preset debug. |
| VS Code/Visual Studio không thấy preset | Mở đúng thư mục gốc có CMakePresets.json. Chạy cmake --list-presets để kiểm tra. Visual Studio cần bật tích hợp CMake Presets và mở lại thư mục. |
| Lỗi C++20 hoặc std::format, ví dụ "format: No such file or directory" | Compiler quá cũ hoặc nhầm compiler, thường là gcc 12 của MinGW, Strawberry Perl hay Anaconda trong PATH. Dùng Visual Studio 2022/2026 hoặc GCC 13 trở lên. |
| "vcpkg triplet ... builds libraries with MSVC, but the compiler is GNU" hoặc "cl.exe not found" | Terminal không có môi trường MSVC. Mở Developer PowerShell for VS, hoặc build trong Visual Studio/VS Code bằng preset vcpkg-debug. |
| Đã đổi compiler/preset mà vẫn lỗi như cũ | CMake nhớ compiler cũ trong cache. Xóa thư mục build\<tên preset> rồi configure lại. |
| Không tạo được OpenGL context | Cập nhật driver đồ họa và chạy ở desktop có hỗ trợ OpenGL. Môi trường remote desktop có thể không cung cấp OpenGL phù hợp. |
| Không đọc được mô hình | Kiểm tra tên/đường dẫn, đuôi .gltf/.glb và các file phụ trợ. Mở Terminal để xem log. |
| DomainDemo mở cửa sổ trống | Demo cần assets/models/Ak_47/Ak-47.obj và các file phụ trợ đúng đường dẫn tương đối từ repo. Dùng InputProcessor để mở GLTF/GLB. |

Hướng dẫn Windows dựa trên CMake Presets dùng chung. Mã nguồn đã được build và chạy trên Linux; hãy chạy configure/build tại máy Windows để xác nhận bộ compiler và driver của máy đó.

Tham khảo tài liệu chính thức: [CMake Tools trong VS Code](https://code.visualstudio.com/docs/cpp/cmake-linux), [CMake Presets trong Visual Studio](https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170), và [vcpkg với Visual Studio](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-vs).
