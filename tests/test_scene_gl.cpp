// Chạy pipeline ECS với OpenGL thật (cần màn hình, dùng xvfb-run trên CI/Linux).
// Kiểm tra: shader của RenderSystem biên dịch được, hình vẽ qua ECS giống hệt shader cũ
// (hiển thị "as it is"), và các công tắc wireframe/texture/lighting thực sự đổi kết quả.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <glad/glad.h>
#include <Application/Application.hpp>
#include <ECS/ECS.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneLoader.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/RenderSystem.hpp>
#include <Scene/Systems/ViewerSystems.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#ifndef TEST_ASSET_DIR
#define TEST_ASSET_DIR "tests/assets"
#endif

namespace {
	namespace ECS = CoreEngine::ECS;
	using namespace Domain::Scene;

	// Shader của viewer trước khi có ECS (examples/gltf_viewer.cpp), dùng làm chuẩn so sánh.
	constexpr auto OLD_VERTEX = R"(#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;
layout(location=3) in vec3 aColor;
uniform mat4 uViewProjection;
out vec3 normal;
out vec2 texCoords;
out vec3 vertexColor;
void main() {
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
    normal = aNormal;
    texCoords = aTexCoords;
    vertexColor = aColor;
})";
	constexpr auto OLD_FRAGMENT = R"(#version 330 core
in vec3 normal;
in vec2 texCoords;
in vec3 vertexColor;
struct Material { sampler2D diffuse[1]; };
uniform Material material;
uniform bool uHasDiffuseTexture;
uniform vec4 uBaseColor;
out vec4 color;
void main() {
    vec4 base = uBaseColor * vec4(vertexColor, 1.0);
    if (uHasDiffuseTexture) base *= texture(material.diffuse[0], texCoords);
    float light = 0.35 + 0.65 * abs(dot(normalize(normal), normalize(vec3(1, 2, 3))));
    color = vec4(base.rgb * light, base.a);
})";

	using Pixels = std::vector<unsigned char>;

	int MaxDifference(const Pixels& a, const Pixels& b) {
		int result = 0;
		for (size_t i = 0; i < a.size(); ++i) result = std::max(result, std::abs(int(a[i]) - int(b[i])));
		return result;
	}

	size_t CountDifferent(const Pixels& a, const Pixels& b) {
		size_t result = 0;
		for (size_t i = 0; i < a.size(); i += 4) {
			if (a[i] != b[i] || a[i + 1] != b[i + 1] || a[i + 2] != b[i + 2]) ++result;
		}
		return result;
	}

	class SceneGlApplication : public CoreEngine::Application {
	public:
		explicit SceneGlApplication(CoreEngine::ApplicationConfiguration& config) : Application(config) {}

		bool Passed = false;

		void OnInitClient() override {
			auto* renderer = GetRenderer();
			auto config = renderer->GetConfig();
			config.DepthOptions.Enabled = true;
			config.DepthOptions.Operation = CoreEngine::DepthOperation::ReadAndWrite;
			config.ClearBufferColor = { 0.08f, 0.09f, 0.12f, 1.0f };
			renderer->Config(config);

			AddViewerSystems(&GetInput(), renderer);
			auto& systems = ECS::SystemManager::GetInstance();
			mRender = systems.GetSystem<RenderSystem>();
			mCameras = systems.GetSystem<CameraSystem>();
			assert(mRender && mRender->IsReady()); // shader thật biên dịch + link thành công
			mCameras->SetViewportSize(config.ViewPortOptions.Width, config.ViewPortOptions.Height);

			mImporter = std::make_unique<Model::AssimpModelImporter>(renderer->GetResourceManager());
			mScene = std::make_unique<Scene>();

			mOldShader = renderer->GetResourceManager()->CreateShaderFromSources(OLD_VERTEX, OLD_FRAGMENT);
			assert(mOldShader && mOldShader->GetShaderId());
			assert(glGetError() == GL_NO_ERROR);
		}

		void OnLoopClient() override {
			auto& settings = mScene->GetRenderSettings();
			ECS::Update(GetTime()->GetDeltaTime());
			assert(glGetError() == GL_NO_ERROR);
			Pixels frame = ReadPixels();

			switch (mFrame) {
			case 0: {
				// Bản B (mỗi node 1 entity, uModel) phải ra cùng ảnh với bản A + shader cũ.
				// Sai số làm tròn số thực cho phép ≤ 2/255 trên từng kênh màu.
				for (const char* path : { TEST_ASSET_DIR "/shear.gltf", TEST_ASSET_DIR "/../../assets/models/Cube/Cube.fbx",
					TEST_ASSET_DIR "/textured.gltf" }) {
					LoadModel(path);
					ECS::Update(0.0f);
					assert(mRender->GetLastDrawCount() >= 1);
					Pixels ecs = ReadPixels();
					assert(CountDifferent(ecs, ClearOnly()) > 100); // model thực sự được vẽ
					Pixels old = RenderWithOldShader(path);
					const int difference = MaxDifference(ecs, old);
					const size_t differentPixels = CountDifferent(ecs, old);
					std::cout << "  " << path << ": max pixel difference ECS vs old = " << difference
						<< " (" << differentPixels << " pixels differ)" << std::endl;
					assert(difference <= 2);
				}
				// textured.gltf được nạp cuối, dùng làm ảnh gốc cho các bước bật/tắt bên dưới.
				ECS::Update(0.0f);
				mNormal = ReadPixels();
				settings.Wireframe = true;
				break;
			}
			case 1:
				assert(CountDifferent(frame, mNormal) > 100); // wireframe khác hình đặc
				settings.Wireframe = false;
				settings.Lighting = false;
				break;
			case 2:
				assert(CountDifferent(frame, mNormal) > 100); // tắt lighting đổi màu
				settings.Lighting = true;
				settings.Textures = false;
				break;
			case 3:
				assert(CountDifferent(frame, mNormal) > 100); // tắt texture đổi màu
				Passed = true;
				RequestClose();
				break;
			}
			++mFrame;
		}

		void OnShutdownClient() override {
			assert(glGetString(GL_VERSION)); // context còn sống khi giải phóng tài nguyên GPU
			mScene.reset();
			ECS::Shutdown();
			assert(glGetError() == GL_NO_ERROR);
		}

	private:
		Pixels ReadPixels() {
			auto viewport = GetRenderer()->GetConfig().ViewPortOptions;
			Pixels pixels(static_cast<size_t>(viewport.Width) * viewport.Height * 4);
			glReadPixels(0, 0, viewport.Width, viewport.Height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
			return pixels;
		}

		Pixels ClearOnly() {
			GetRenderer()->GetRendererCommand()->ClearBuffers(CoreEngine::ClearBufferMasks::Color | CoreEngine::ClearBufferMasks::Depth);
			return ReadPixels();
		}

		void LoadModel(const char* path) {
			SceneLoader loader(*mImporter);
			Actor root = loader.LoadModel(*mScene, path);
			assert(root.IsValid());
			mCameras->CreateCamera(*mScene, &root.GetComponent<BoundsComponent>()->Box);
		}

		// Bản A: đỉnh đã áp node transform, vẽ bằng shader trước khi có ECS.
		Pixels RenderWithOldShader(const char* path) {
			auto* renderer = GetRenderer();
			auto flat = mImporter->Import(path);
			assert(flat);
			renderer->GetRendererCommand()->ClearBuffers(CoreEngine::ClearBufferMasks::Color | CoreEngine::ClearBufferMasks::Depth);
			CameraComponent* camera = CameraSystem::GetActiveCamera();
			const glm::mat4 viewProjection = camera->Camera.GetProjectionMatrix() * camera->Camera.GetViewMatrix();
			mOldShader->Use();
			mOldShader->SetUniformMatrix4fv("uViewProjection", glm::value_ptr(viewProjection));
			renderer->GetRendererCommand()->SetPolygonMode(CoreEngine::PolygonMode::Fill);
			flat->Render(renderer, mOldShader);
			return ReadPixels();
		}

	private:
		RenderSystem* mRender = nullptr;
		CameraSystem* mCameras = nullptr;
		std::unique_ptr<Model::AssimpModelImporter> mImporter;
		std::unique_ptr<Scene> mScene;
		CoreEngine::IShader* mOldShader = nullptr;
		Pixels mNormal;
		int mFrame = 0;
	};
}

int main() {
	CoreEngine::ApplicationConfiguration config{ 320, 240, "scene gl check" };
	config.MaxFPS = 100;
	SceneGlApplication app(config);
	app.Run();
	assert(app.Passed);
	std::cout << "ECS render pipeline (real OpenGL) PASS\n";
	return 0;
}
