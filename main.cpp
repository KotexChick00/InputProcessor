// Viewer mô hình 3D chạy trên ECS (Core/ECS + Domain/Scene).
//
//   SceneLoader     đọc file (glTF/GLB/OBJ/FBX qua Assimp) thành entity: model + các đèn (hoặc đèn mặc định)
//   CameraSystem    camera orbit nhìn bao trọn model, điều khiển bằng chuột/bàn phím
//   LightingSystem  gom đèn cho shader
//   RenderSystem    vẽ model theo tuỳ chọn hiển thị (wireframe / texture / lighting)
//
// Cách dùng: DomainDemo [model] [--frames N]
//   --frames N: tự đóng sau N frame (dùng cho smoke test).

#include <Application/Application.hpp>
#include <ECS/ECS.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneLoader.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/ViewerSystems.hpp>

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
		ModelViewer(CoreEngine::ApplicationConfiguration& config, std::string path, int frameLimit)
			: Application(config), mPath(std::move(path)), mFrameLimit(frameLimit) {}

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
			if (!OpenModel(mPath)) throw std::runtime_error("Failed to read model: " + mPath);
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
				<< "  Esc                     quit\n";
		}

	private:
		std::string mPath;
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
		int frameLimit = 0;
		bool hasPath = false;
		for (int i = 1; i < argc; ++i) {
			std::string_view argument = argv[i];
			if (argument == "--frames" && i + 1 < argc) {
				frameLimit = ParseFrameCount(argv[++i]);
			} else if (!hasPath && !argument.starts_with("--")) {
				path = argv[i];
				hasPath = true;
			} else {
				std::cerr << "Usage: DomainDemo [model.gltf|.glb|.obj|.fbx] [--frames N]\n";
				return 1;
			}
		}

		CoreEngine::ApplicationConfiguration config{ 1000, 700, "InputProcessor - Model Viewer" };
		ModelViewer viewer(config, path, frameLimit);
		viewer.Run();
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
	return 0;
}
