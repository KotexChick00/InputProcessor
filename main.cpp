// Viewer mô hình 3D chạy trên ECS (Core/ECS + Domain/Scene).
//
//   ViewerController  API của viewer (mở model, lưu/nạp trạng thái xem, cây node, bật tắt) — GUI dùng lớp này
//   SceneLoader     đọc file (glTF/GLB/OBJ/FBX qua Assimp) thành entity: mỗi node 1 entity, đèn, đèn mặc định
//   CameraSystem    camera orbit nhìn bao trọn model, điều khiển bằng chuột/bàn phím
//   LightingSystem  gom đèn cho shader
//   RenderSystem    vẽ model theo tuỳ chọn hiển thị (wireframe / texture / lighting)
//
// Cách dùng: DomainDemo [model] [--view FILE] [--frames N]
//   --view FILE: file trạng thái xem (JSON). Có sẵn thì mở theo nó; F5 lưu vào đây, F9 nạp lại.
//                Mặc định: <tên model>.view.json trong thư mục hiện tại.
//   --frames N:  tự đóng sau N frame (dùng cho smoke test).

#include <Application/Application.hpp>
#include <ECS/ECS.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/ViewerSystems.hpp>
#include <Scene/ViewerController.hpp>
#include <Scene/Utf8Path.hpp>

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef INPUTPROCESSOR_SAMPLE_MODEL
#define INPUTPROCESSOR_SAMPLE_MODEL "tests/assets/textured.gltf"
#endif

namespace {
	namespace ECS = CoreEngine::ECS;
	using CoreEngine::KeyboardKey;
	using namespace Domain::Scene;

	class ModelViewer : public CoreEngine::Application {
	public:
		ModelViewer(CoreEngine::ApplicationConfiguration& config, std::string path, std::string viewFile, int frameLimit)
			: Application(config), mPath(std::move(path)), mViewFile(std::move(viewFile)), mFrameLimit(frameLimit) {}

		void OnInitClient() override {
			auto* renderer = GetRenderer();
			auto config = renderer->GetConfig();
			config.DepthOptions.Enabled = true;
			config.DepthOptions.Operation = CoreEngine::DepthOperation::ReadAndWrite;
			config.ClearBufferColor = { 0.08f, 0.09f, 0.12f, 1.0f };
			renderer->Config(config);

			AddViewerSystems(&GetInput(), renderer);
			auto* cameras = ECS::SystemManager::GetInstance().GetSystem<CameraSystem>();
			cameras->SetViewportSize(config.ViewPortOptions.Width, config.ViewPortOptions.Height);
			GetEventDispatcher()->AddEventListener<CoreEngine::WindowResizeEventContext>([cameras](const auto& event) {
				cameras->SetViewportSize(event.GetWidth(), event.GetHeight());
				return false;
			});
			mViewer = std::make_unique<ViewerController>(renderer->GetResourceManager(), cameras);

			// Menu "File -> Open Model..." (Ctrl+O): cửa sổ chỉ phát yêu cầu mở file,
			// hộp thoại và việc mở model nằm ở đây (tầng ứng dụng) — Core không biết Domain.
			GetWindow()->OnOpenFileRequestCallback([this]() { PromptOpenModel(); });

			// Có file trạng thái xem thì mở theo nó (model + camera + tuỳ chọn), không thì mở model.
			std::error_code code;
			if (!mViewFile.empty() && std::filesystem::exists(PathFromUtf8(mViewFile), code)) {
				if (!LoadView()) throw std::runtime_error("Failed to open view: " + mViewFile);
			} else if (!OpenModel(mPath)) {
				throw std::runtime_error("Failed to read model: " + mPath);
			}
			if (mViewFile.empty()) mViewFile = PathToUtf8(PathFromUtf8(mPath).stem()) + ".view.json";
			PrintControls();
		}

		void OnLoopClient() override {
			if (auto* keyboard = GetInput().KeyboardInput) {
				// JustPressed: một cú bấm Esc nhanh (nhấn + nhả trong cùng frame) cũng đóng được.
				if (keyboard->CheckIsJustPressed(KeyboardKey::Escape) || keyboard->CheckIsPressed(KeyboardKey::Escape)) RequestClose();
				if (keyboard->CheckIsJustPressed(KeyboardKey::R)) mViewer->FitCamera();

				auto& settings = mViewer->GetRenderSettings();
				if (keyboard->CheckIsJustPressed(KeyboardKey::One)) Toggle("Wireframe", settings.Wireframe);
				if (keyboard->CheckIsJustPressed(KeyboardKey::Two)) Toggle("Textures", settings.Textures);
				if (keyboard->CheckIsJustPressed(KeyboardKey::Three)) Toggle("Lighting", settings.Lighting);
				if (keyboard->CheckIsJustPressed(KeyboardKey::F5)) SaveView();
				if (keyboard->CheckIsJustPressed(KeyboardKey::F9)) LoadView();
			}

			ECS::Update(GetTime()->GetDeltaTime());

			if (mFrameLimit > 0 && ++mFrames >= mFrameLimit) RequestClose();
		}

		void OnShutdownClient() override {
			// Buffer GPU của model và shader cần OpenGL context: huỷ scene + ECS trước khi đóng cửa sổ.
			mViewer.reset();
			ECS::Shutdown();
		}

	private:
		// Mở model thay cho model hiện tại; lỗi thì giữ nguyên cảnh cũ.
		bool OpenModel(const std::string& path) {
			std::string error;
			if (!mViewer->OpenModel(path, error)) {
				std::cerr << error << '\n';
				return false;
			}
			PrintStats();
			return true;
		}

		// Hộp thoại chọn model: hủy hoặc file lỗi thì giữ nguyên cảnh hiện tại. Mở được thì
		// chuyển sang file trạng thái xem riêng của model mới (<tên model>.view.json),
		// đúng quy ước như khi mở từ dòng lệnh; thư mục mở dialog là thư mục model hiện tại.
		void PromptOpenModel() {
			CoreEngine::WindowFileDialogOptions options;
			options.Title = "Open model";
			options.Filters = "3D models (*.gltf;*.glb;*.obj;*.fbx)|*.gltf;*.glb;*.obj;*.fbx"
				"|GLTF/GLB (*.gltf;*.glb)|*.gltf;*.glb"
				"|Wavefront OBJ (*.obj)|*.obj"
				"|FBX (*.fbx)|*.fbx"
				"|All files (*.*)|*.*";
			if (!mPath.empty()) options.StartDirectory = PathToUtf8(PathFromUtf8(mPath).parent_path());

			std::string path;
			if (!GetWindow()->ShowOpenFileDialog(options, path)) return;
			if (OpenModel(path)) {
				mPath = path;
				mViewFile = PathToUtf8(PathFromUtf8(path).stem()) + ".view.json";
			}
		}

		// Lưu trạng thái xem (không đụng tới file model).
		void SaveView() {
			std::string error;
			if (mViewer->SaveView(mViewFile, error)) std::cout << "Saved view to " << mViewFile << '\n';
			else std::cerr << "Save view failed: " << error << '\n';
		}

		// Nạp trạng thái xem: mở model ghi trong file rồi áp camera/tuỳ chọn. Lỗi thì giữ nguyên cảnh hiện tại.
		bool LoadView() {
			std::string error;
			if (!mViewer->LoadView(mViewFile, error)) {
				std::cerr << "Load view failed: " << error << '\n';
				return false;
			}
			mPath = mViewer->GetModelPath();
			std::cout << "Loaded view from " << mViewFile << '\n';
			PrintStats();
			return true;
		}

		void PrintStats() const {
			const ModelStats stats = mViewer->GetModelStats();
			std::cout << "Opened " << stats.Path << ": " << stats.NodeCount << " nodes, " << stats.MeshCount << " meshes, "
				<< stats.VertexCount << " vertices, " << stats.TriangleCount << " triangles, " << stats.LightCount << " lights\n";
			if (!stats.HasBounds) std::cerr << "Warning: the model has no vertices, nothing to show\n";
		}

		static void Toggle(const char* name, bool& value) {
			value = !value;
			std::cout << name << ": " << (value ? "on" : "off") << '\n';
		}

		static void PrintControls() {
			std::cout << "Controls\n"
				<< "  File menu / Ctrl+O      open another model\n"
				<< "  Left drag / arrows      orbit\n"
				<< "  Right drag / W A S D    pan\n"
				<< "  Middle drag / PgUp PgDn dolly\n"
				<< "  Mouse wheel             zoom\n"
				<< "  R                       fit camera to the model\n"
				<< "  1 / 2 / 3               toggle wireframe / textures / lighting\n"
				<< "  F5 / F9                 save / load the view (camera, options) as JSON\n"
				<< "  Esc                     quit\n";
		}

	private:
		std::string mPath;
		std::string mViewFile;
		int mFrameLimit = 0;
		int mFrames = 0;
		std::unique_ptr<ViewerController> mViewer;
	};

	int ParseFrameCount(std::string_view text) {
		std::string count(text);
		size_t end = 0;
		int frames = std::stoi(count, &end);
		if (end != count.size() || frames <= 0) throw std::invalid_argument("Frame count must be a positive integer");
		return frames;
	}
}

int main(int argc, char** argv) {
	try {
		std::string path = INPUTPROCESSOR_SAMPLE_MODEL;
		std::string viewFile;
		int frameLimit = 0;
		bool hasPath = false;
		for (int i = 1; i < argc; ++i) {
			std::string_view argument = argv[i];
			if (argument == "--frames" && i + 1 < argc) {
				frameLimit = ParseFrameCount(argv[++i]);
			} else if (argument == "--view" && i + 1 < argc) {
				viewFile = argv[++i];
			} else if (!hasPath && !argument.starts_with("--")) {
				path = argv[i];
				hasPath = true;
			} else {
				std::cerr << "Usage: DomainDemo [model.gltf|.glb|.obj|.fbx] [--view FILE.json] [--frames N]\n";
				return 1;
			}
		}

		CoreEngine::ApplicationConfiguration config{ 1000, 700, "InputProcessor - Model Viewer" };
		ModelViewer viewer(config, path, viewFile, frameLimit);
		viewer.Run();
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
	return 0;
}
