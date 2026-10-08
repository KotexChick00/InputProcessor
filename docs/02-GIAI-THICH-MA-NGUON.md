# Giải thích cấu trúc và luồng chạy của mã nguồn

Tài liệu này đi theo thứ tự chương trình khởi động, đọc mô hình và vẽ ra màn hình. Có thể đọc từ đầu; không cần biết trước OpenGL hoặc CMake.

## 1. Mỗi mã nguồn tạo ra chương trình nào?

CMakeLists.txt khai báo các target. Target là tên chương trình hoặc bộ kiểm tra mà CMake có thể build.

| Target | File bắt đầu | Chức năng |
|---|---|---|
| InputProcessor | [examples/gltf_viewer.cpp](../examples/gltf_viewer.cpp) | Viewer chính, nhận GLTF hoặc GLB. |
| DomainDemo | [main.cpp](../main.cpp) | Demo OBJ cũ của nhóm, cần assets/models/Ak_47/Ak-47.obj. |
| test_engine_logic | tests/test_engine_logic.cpp | Kiểm tra camera, input và xử lý dữ liệu model. |
| test_wx_integration | tests/test_wx_integration.cpp | Kiểm tra kết hợp cửa sổ, sự kiện và OpenGL. |

Thư mục src chứa các file AsciiReader và FormatChecker từ phần thử nghiệm trước. Chúng không nằm trong danh sách nguồn của target InputProcessor ở CMakeLists.txt, nên không phải điểm bắt đầu của viewer. Library/Glad chứa mã GLAD dùng để tải các hàm OpenGL.

## 2. Bản đồ thư mục

| File/thư mục | Vai trò |
|---|---|
| CMakeLists.txt | Khai báo thư viện, file nguồn, target và CTest. |
| CMakePresets.json | Cấu hình Debug/Release dùng chung cho IDE và Terminal. |
| vcpkg.json | Danh sách dependency do vcpkg cài. |
| Core/Application | Tạo cửa sổ/renderer và chạy vòng lặp ứng dụng. |
| Core/Window | Interface cửa sổ, factory, backend wxWidgets và GLFW. Viewer mặc định dùng wxWidgets. |
| Core/Input | Lưu trạng thái phím, chuột, sự kiện và chuyển động theo từng khung hình. |
| Core/Renderer | Interface vẽ và phần triển khai OpenGL: shader, buffer, texture, framebuffer. |
| Domain/Model | Mesh, vật liệu, mô hình, importer và bounding box. |
| Domain/Camera/OrbitCamera | Camera và các state điều khiển bằng chuột hoặc phím. |
| examples/gltf_viewer.cpp | Entry point và hành vi của viewer GLTF/GLB. |
| tests/assets | Mô hình nhỏ để chạy ví dụ và kiểm tra mà không cần tải file bên ngoài. |

## 3. Luồng tổng thể

~~~mermaid
flowchart TD
    A[Chạy InputProcessor] --> B[Chọn file hoặc model mẫu]
    B --> C[Khởi tạo wxWidgets và OpenGL]
    C --> D[Assimp đọc GLTF hoặc GLB]
    D --> E[Tạo RenderModel từ mesh và vật liệu]
    E --> F[Căn camera theo kích thước model]
    F --> G{Lặp lại mỗi khung hình}
    G --> H[Nhận sự kiện bàn phím và chuột]
    H --> I[Cập nhật camera]
    I --> J[Tạo buffer GPU lần đầu cần vẽ]
    J --> K[Shader vẽ các mesh]
    K --> L[Hiện khung hình trong cửa sổ]
    L --> G
~~~

GPU là bộ phận của card đồ họa dùng để vẽ. Khung hình là một ảnh hoàn chỉnh trong cửa sổ; engine lặp lại việc vẽ nhiều lần mỗi giây.

## 4. Từ nút Run đến cửa sổ

Điểm bắt đầu của viewer là hàm main trong examples/gltf_viewer.cpp:

1. Không có đối số: mở tests/assets/textured.gltf.
2. Có đường dẫn: dùng file GLTF/GLB được truyền vào.
3. Có tùy chọn --frames N: tự đóng sau N khung hình. CTest dùng tuỳ chọn này.
4. Tạo ModelViewer rồi gọi Run().

ModelViewer kế thừa CoreEngine::Application. Application::Init() tạo logger, hỏi WindowPlatformFactory để tạo cửa sổ, khởi động wxWidgets, lấy input, tạo renderer OpenGL và đăng ký callback sự kiện. Cấu hình mặc định chọn wx; mã backend GLFW vẫn có thể được chọn ở những nơi cấu hình tương ứng.

Sau khi khởi tạo engine, OnInitClient() chạy một lần. Nó bật kiểm tra độ sâu để tam giác gần che tam giác xa, đặt màu nền, tạo shader GLSL, đọc model và căn camera.

## 5. Cách file model được đọc

GltfModelFormatHandler kiểm tra phần mở rộng .gltf/.glb rồi gọi AssimpModelImporter. Assimp là thư viện đọc nhiều định dạng mô hình 3D. Import handler có thể nối tiếp nhau để thêm định dạng, nhưng viewer hiện chỉ khởi tạo handler GLTF; vì vậy InputProcessor mở GLTF/GLB, không phải mọi định dạng Assimp hỗ trợ.

AssimpModelImporter đọc cảnh 3D và xử lý theo từng node:

1. Đi qua node cha/con và áp dụng transform của node lên mesh.
2. Với mỗi vertex, lưu vị trí, normal, tọa độ UV và màu.
3. Chuyển mặt đa giác thành tam giác và lấy danh sách index.
4. Đọc màu và texture của vật liệu. Texture có thể nằm ngoài file model hoặc được nhúng vào GLB.
5. Tạo RenderModel chứa các mesh và vật liệu.

Normal mô tả hướng của bề mặt để tính ánh sáng. UV cho biết phần nào của ảnh texture được dán lên bề mặt. Transform là phép xoay/tịnh tiến/tỉ lệ mà file GLTF gán cho từng node.

Viewer đặt shader ngay trong examples/gltf_viewer.cpp. Shader đọc vertex, màu vật liệu và diffuse texture rồi tính ánh sáng đơn giản. Viewer tập trung vào model tĩnh; animation, skinning và pipeline PBR hoàn chỉnh chưa thuộc phạm vi này.

## 6. Từ dữ liệu model tới GPU

RenderModel ban đầu giữ dữ liệu ở bộ nhớ máy tính. Khi RenderModel::Render() được gọi lần đầu, nó gọi UploadToGPU():

1. Gom vị trí, normal, UV, màu và index từ các mesh.
2. Dùng resource manager để tạo vertex buffer và index buffer.
3. Gắn vật liệu tương ứng với từng phần mesh.
4. Gọi lệnh OpenGL vẽ các tam giác.

Upload được thực hiện khi cần vẽ lần đầu. Các lần vẽ sau tái sử dụng buffer. Resource manager quản lý shader, texture và các tài nguyên dùng chung. Texture nhúng được truyền dưới dạng byte tới CreateTextureFromMemory(); texture ngoài được tìm tương đối từ thư mục của model.

Nếu GLTF tham chiếu đến một file .bin hoặc texture không có, importer không thể dựng lại model đầy đủ. Khi chia sẻ một GLTF, hãy gửi cả các file phụ trợ và giữ nguyên cấu trúc thư mục tương đối.

## 7. Cách engine vẽ mỗi khung hình

Application::Run() gọi các bước này lặp lại:

1. BeginLoop() tính thời gian của khung hình và gọi PollEvents() để wxWidgets nhận sự kiện cửa sổ/input.
2. OnLoopClient() của ModelViewer xử lý Esc và R, cập nhật camera bằng OrbitCameraController.
3. Viewer tạo ma trận camera và shader, rồi xóa bộ đệm màu/độ sâu của khung hình trước.
4. RenderModel vẽ từng phần mesh.
5. EndLoop() xử lý sự kiện cuối khung hình và SwapBuffers() hiện ảnh mới trong cửa sổ.

Nút resize cập nhật kích thước viewport và camera. Mất focus/capture xóa trạng thái phím/nút để một thao tác bị ngắt không giữ camera mãi.

Khi thoát, viewer xóa controller và model trước. Application sau đó hủy renderer khi ngữ cảnh OpenGL còn mở, rồi mới đóng cửa sổ. Trình tự này giúp tài nguyên GPU được giải phóng đúng lúc.

## 8. Camera và state điều khiển

OrbitCamera giữ điểm mà camera nhìn vào, góc xoay, khoảng cách, phép chiếu và kích thước viewport. OrbitCameraController đọc hành động từ InputMap rồi chuyển giữa các state:

| State | Công việc | Input mặc định |
|---|---|---|
| IdleState | Chờ thao tác. | Không có gesture nào đang giữ. |
| OrbitingState | Xoay quanh mục tiêu. | Giữ chuột trái và kéo; hoặc phím mũi tên. |
| PanningState | Dời điểm nhìn trên mặt phẳng camera. | Giữ chuột phải và kéo; hoặc W/A/S/D. |
| DollyingState | Thay đổi khoảng cách nhìn. | Giữ chuột giữa và kéo; hoặc PageUp/PageDown. |

Gesture là một thao tác đang diễn ra, ví dụ đang giữ chuột phải hoặc phím W. InputMap cho phép đổi phím gắn với từng hành động. Khi điều khiển bằng chuột, camera dùng khoảng cách con trỏ đã di chuyển; khi điều khiển bằng phím, nó dùng thời gian đã trôi qua để tốc độ ổn định giữa các máy.

Viewer tính bounding box từ vertex đã áp dụng transform. FitCamera dùng kích thước đó để đặt tâm nhìn, khoảng cách zoom và mặt phẳng clipping. Vì vậy model lớn/nhỏ thường được đặt vừa màn hình khi mở và khi bấm R.

## 9. Nếu muốn sửa một phần chương trình

| Muốn thay đổi | Nơi bắt đầu |
|---|---|
| Cửa sổ hoặc vòng lặp ứng dụng | Core/Application/Application.cpp và Core/Window/Wx/WxWindow.cpp |
| Cách wxWidgets ghi input | Core/Input/Wx/ |
| Thêm định dạng model | Domain/Model/Importers/ModelImporter/ và AssimpModelImporter/ |
| Cách model được lưu/vẽ | Domain/Model/RenderModel.cpp và Core/Renderer/Opengl/ |
| Shader của viewer | Hai chuỗi GLSL trong examples/gltf_viewer.cpp |
| Điều khiển và phím camera | Domain/Camera/InputMap.hpp, OrbitCameraAction.hpp, OrbitCamera/States/ |
| Thêm target hoặc dependency | CMakeLists.txt, CMakePresets.json, vcpkg.json |

Build target InputProcessor để chạy viewer. Làm theo [hướng dẫn cài đặt và chạy](01-HUONG-DAN-CAI-DAT-VA-CHAY.md) nếu cần biết chọn preset trong IDE.

## 10. Từ điển nhỏ

| Từ | Nghĩa dễ hiểu |
|---|---|
| Compiler (trình biên dịch) | Chuyển C++ thành chương trình máy tính có thể chạy. |
| CMake | Đọc CMakeLists.txt và tạo ra cấu hình build phù hợp compiler/IDE. |
| Preset | Cấu hình CMake có tên, ví dụ vcpkg-debug, để các máy dùng cùng cách build. |
| Dependency | Thư viện bên ngoài mà chương trình dùng, ví dụ Assimp để đọc model. |
| Mesh | Các đỉnh và tam giác tạo thành một phần của model. |
| Texture | Ảnh được gắn lên bề mặt model. |
| Shader | Chương trình nhỏ chạy trên GPU để tính màu khi vẽ. |
| State machine | Cách chia hành vi thành trạng thái, ở đây gồm chờ, xoay, pan và dolly. |
