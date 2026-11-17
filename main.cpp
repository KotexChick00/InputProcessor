// Viewer mô hình 3D chạy trên ECS (Core/ECS + Domain/Scene).
//
//   SceneLoader     đọc file (glTF/GLB/OBJ/FBX qua Assimp) thành entity: model + các đèn (hoặc đèn mặc định)
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
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneLoader.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/ViewerSystems.hpp>
#include <Scene/ViewState.hpp>

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
			mCameras = ECS::SystemManager::GetInstance().GetSystem<CameraSystem>();
			mCameras->SetViewportSize(config.ViewPortOptions.Width, config.ViewPortOptions.Height);
			GetEventDispatcher()->AddEventListener<CoreEngine::WindowResizeEventContext>([this](const auto& event) {
				mCameras->SetViewportSize(event.GetWidth(), event.GetHeight());
				return false;
			});

			mImporter = std::make_unique<Model::AssimpModelImporter>(renderer->GetResourceManager());
			mScene = std::make_unique<Scene>();
			// Có file trạng thái xem thì mở theo nó (model + camera + tuỳ chọn), không thì mở model.
			std::error_code code;
			if (!mViewFile.empty() && std::filesystem::exists(mViewFile, code)) {
				if (!LoadView()) throw std::runtime_error("Failed to open view: " + mViewFile);
			} else if (!OpenModel(mPath)) {
				throw std::runtime_error("Failed to read model: " + mPath);
			}
			if (mViewFile.empty()) mViewFile = std::filesystem::path(mPath).stem().string() + ".view.json";
			PrintControls();
		}

		void OnLoopClient() override {
			const auto& input = GetInput();
			auto* keyboard = input.KeyboardInput;
			if (keyboard) {
				if (keyboard->CheckIsPressed(KeyboardKey::Escape)) RequestClose();
				if (keyboard->CheckIsJustPressed(KeyboardKey::R)) FitCamera();

				auto& settings = mScene->GetRenderSettings();
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
			mScene.reset();
			ECS::Shutdown();
			mImporter.reset();
			mCameras = nullptr;
		}

	private:
		// Mở model thay cho model hiện tại; lỗi thì giữ nguyên scene cũ.
		bool OpenModel(const std::string& path) {
			SceneLoader loader(*mImporter);
			mModel = loader.LoadModel(*mScene, path);
			if (!mModel.IsValid()) return false;

			auto* bounds = mModel.GetComponent<BoundsComponent>();
			if (!bounds || !bounds->Box.isValid()) {
				std::cerr << "Warning: '" << path << "' has no vertices, nothing to show\n";
			}
			mCameras->CreateCamera(*mScene, bounds ? &bounds->Box : nullptr);
			std::cout << "Opened " << path << " (" << mScene->GetEntities().size() << " entities)\n";
			return true;
		}

		// Lưu trạng thái xem (không đụng tới file model).
		bool SaveView() {
			std::string error;
			ViewState state = CaptureViewState(*mScene, mModel, CameraSystem::GetActiveCamera(), mViewFile);
			if (!SaveViewState(state, mViewFile, error)) {
				std::cerr << "Save view failed: " << error << '\n';
				return false;
			}
			std::cout << "Saved view to " << mViewFile << '\n';
			return true;
		}

		// Nạp trạng thái xem: mở model ghi trong file rồi áp camera/tuỳ chọn. Lỗi thì giữ nguyên cảnh hiện tại.
		bool LoadView() {
			std::string error;
			auto state = LoadViewState(mViewFile, error);
			if (!state) {
				std::cerr << "Load view failed: " << error << '\n';
				return false;
			}
			const std::string model = ResolveModelPath(*state, mViewFile).string();
			if (!OpenModel(model)) {
				std::cerr << "Load view failed: can't open model '" << model << "'\n";
				return false;
			}
			mPath = model;
			const size_t skipped = ApplyViewState(*state, *mScene, mModel, CameraSystem::GetActiveCamera());
			std::cout << "Loaded view from " << mViewFile;
			if (skipped > 0) std::cout << " (" << skipped << " entries not found in the model)";
			std::cout << '\n';
			return true;
		}

		void FitCamera() {
			CameraComponent* camera = CameraSystem::GetActiveCamera();
			auto* bounds = mModel.IsValid() ? mModel.GetComponent<BoundsComponent>() : nullptr;
			if (camera && bounds) mCameras->FitToBounds(*camera, bounds->Box);
		}

		static void Toggle(const char* name, bool& value) {
			value = !value;
			std::cout << name << ": " << (value ? "on" : "off") << '\n';
		}

		static void PrintControls() {
			std::cout << "Controls\n"
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

		std::unique_ptr<Model::AssimpModelImporter> mImporter;
		std::unique_ptr<Scene> mScene;
		Actor mModel;
		CameraSystem* mCameras = nullptr; // Thuộc SystemManager
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
