// Giữ assert hoạt động cả khi build Release.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <Scene/Scene.hpp>
#include <Scene/SceneLoader.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Renderer/Resource/IResourceManager.hpp>
#include <Scene/Systems/TransformSystem.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/LightingSystem.hpp>
#include <Scene/Systems/RenderSystem.hpp>
#include <Scene/Systems/ViewerSystems.hpp>
#include <Scene/ViewState.hpp>
#include <Input/Wx/WxKeyboardInput.hpp>
#include <Input/Wx/WxMouseInput.hpp>

#include <map>
#include <string>
#include <vector>

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#ifndef TEST_ASSET_DIR
#define TEST_ASSET_DIR "tests/assets"
#endif

using namespace Domain::Scene;
namespace ECS = CoreEngine::ECS;

namespace {
	bool Vec3Equal(const glm::vec3& a, const glm::vec3& b, float eps = 1e-3f) {
		return glm::length(a - b) < eps;
	}

	std::string Asset(const char* name) {
		return (std::filesystem::path(TEST_ASSET_DIR) / name).string();
	}

	size_t AliveEntities() {
		return ECS::EntityManager::GetInstance().GetAliveCount();
	}

	// Resource manager giả: importer chỉ cần tạo texture, không cần OpenGL.
	class TestTexture : public CoreEngine::ITexture {
	public:
		void Config(const CoreEngine::TextureConfiguration&) override {}
		CoreEngine::TextureID GetTextureId() const override { return 1; }
	};

	class TestResources : public CoreEngine::IResourceManager {
	public:
		TestTexture texture;
		CoreEngine::IVertexBuffer* GetVertexBuffer(unsigned int) override { return nullptr; }
		CoreEngine::IIndexBuffer* GetIndexBuffer(unsigned int) override { return nullptr; }
		CoreEngine::IShader* GetShader(unsigned int) override { return nullptr; }
		CoreEngine::ITexture* GetTexture(unsigned int) override { return nullptr; }
		CoreEngine::ICubeMap* GetCubeMap(unsigned int) override { return nullptr; }
		CoreEngine::IUniformBuffer* GetUniformBuffer(unsigned int) override { return nullptr; }
		CoreEngine::IVertexBuffer* CreateVertexBuffer() override { return nullptr; }
		CoreEngine::IIndexBuffer* CreateIndexBuffer() override { return nullptr; }
		CoreEngine::IShader* CreateShaderFromSources(const std::string&, const std::string&) override { return nullptr; }
		CoreEngine::IShader* CreateShaderFromFiles(const std::string&, const std::string&) override { return nullptr; }
		CoreEngine::ICubeMap* CreateCubeMap(const CoreEngine::CubemapTextureFiles&) override { return nullptr; }
		CoreEngine::IUniformBuffer* CreateUniformBuffer() override { return nullptr; }
		CoreEngine::IFrameBuffer* CreateColorFrameBuffer(unsigned int, unsigned int) override { return nullptr; }
		CoreEngine::ITexture* CreateTexture(const std::string&) override { return &texture; }
		CoreEngine::ITexture* CreateTextureFromMemory(const unsigned char*, unsigned int) override { return &texture; }
	};

	// --- Renderer giả: ghi lại lệnh vẽ và uniform để kiểm tra RenderSystem không cần OpenGL ---
	class FakeShader : public CoreEngine::IShader {
	public:
		std::map<std::string, std::vector<float>> Uniforms;
		int UseCount = 0;
		void SetUniformMatrix4fv(const std::string& name, const float* value) override { Uniforms[name].assign(value, value + 16); }
		void SetUniformMatrix3fv(const std::string& name, const float* value) override { Uniforms[name].assign(value, value + 9); }
		void SetUniform1i(const std::string& name, int value) override { Uniforms[name] = { static_cast<float>(value) }; }
		void SetUniform1f(const std::string& name, float value) override { Uniforms[name] = { value }; }
		void SetUniform2f(const std::string& name, float a, float b) override { Uniforms[name] = { a, b }; }
		void SetUniform3f(const std::string& name, float a, float b, float c) override { Uniforms[name] = { a, b, c }; }
		void SetUniform4f(const std::string& name, float a, float b, float c, float d) override { Uniforms[name] = { a, b, c, d }; }
		void SetUniformBuffer(const std::string&, unsigned int) override {}
		void SetUniformTexture(const std::string& name, CoreEngine::ITexture*) override { Uniforms[name] = { 1 }; }
		void Use() override { ++UseCount; }
		void Unuse() override {}
		CoreEngine::ShaderID GetShaderId() const override { return 7; }
		float Get(const std::string& name) const { auto it = Uniforms.find(name); return it == Uniforms.end() ? -999.0f : it->second.front(); }
	};

	class FakeVertexBuffer : public CoreEngine::IVertexBuffer {
	public:
		void SetData(unsigned int, void*, unsigned int, unsigned int, unsigned int, CoreEngine::VertexBufferDataType, CoreEngine::VertexBufferRenderMode) override {}
		void SetData(unsigned int, void*, unsigned int, unsigned int, unsigned int, unsigned int, CoreEngine::VertexBufferDataType, CoreEngine::VertexBufferRenderMode) override {}
		void Bind() override {}
		void UnBind() override {}
		void EnableAttrib(unsigned int) override {}
		void DisableAttrib(unsigned int) override {}
		CoreEngine::VertexBufferID GetVertexBufferId() const override { return 1; }
	};

	class FakeIndexBuffer : public CoreEngine::IIndexBuffer {
	public:
		void SetData(unsigned int*, unsigned int, CoreEngine::IndexBufferRenderMode) override {}
		void Bind() override {}
		void UnBind() override {}
		CoreEngine::IndexBufferID GetIndexBufferId() const override { return 1; }
	};

	// RenderModel tự delete buffer nó tạo, nên ở đây cấp phát mới mỗi lần.
	class FakeGpuResources : public TestResources {
	public:
		FakeShader Shader;
		int VertexBuffersCreated = 0;
		CoreEngine::IVertexBuffer* CreateVertexBuffer() override { ++VertexBuffersCreated; return new FakeVertexBuffer(); }
		CoreEngine::IIndexBuffer* CreateIndexBuffer() override { return new FakeIndexBuffer(); }
		CoreEngine::IShader* CreateShaderFromSources(const std::string& vertex, const std::string& fragment) override {
			assert(vertex.find("uModel") != std::string::npos && fragment.find("uLights") != std::string::npos);
			return &Shader;
		}
	};

	class FakeCommand : public CoreEngine::IRendererCommand {
	public:
		int Clears = 0;
		std::vector<unsigned int> DrawnIndexCounts;
		std::vector<CoreEngine::PolygonMode> PolygonModes;
		void ClearBuffers(CoreEngine::ClearBufferMasks) override { ++Clears; }
		void Draw(CoreEngine::RenderMode, unsigned int, unsigned int) override {}
		void DrawIndex(CoreEngine::RenderMode, unsigned int count, unsigned int) override { DrawnIndexCounts.push_back(count); }
		void SetPolygonMode(CoreEngine::PolygonMode mode) override { PolygonModes.push_back(mode); }
	};

	class FakeRenderer : public CoreEngine::IRenderer {
	public:
		FakeCommand Command;
		FakeGpuResources Resources;
		void Config(CoreEngine::RendererConfiguration) override {}
		CoreEngine::IRendererCommand* GetRendererCommand() override { return &Command; }
		CoreEngine::IResourceManager* GetResourceManager() override { return &Resources; }
		CoreEngine::IFrameBufferManager* GetFrameBufferManager() override { return nullptr; }
		void Free() override {}
		CoreEngine::RendererConfiguration GetConfig() override { return {}; }
	};
}

void TestHierarchyAndDestroy() {
	std::cout << "[RUN] Scene hierarchy and destroy...\n";
	{
		Scene scene;
		const size_t base = AliveEntities(); // chỉ có Render Settings
		Actor root = scene.CreateEntity("Root");
		Actor child = scene.CreateEntity("Child", root);
		Actor grandChild = scene.CreateEntity("GrandChild", child);
		Actor other = scene.CreateEntity("Other");

		assert(AliveEntities() == base + 4);
		assert(scene.GetEntities().size() == 4);
		assert(scene.GetRoots().size() == 2);
		assert(child.GetComponent<HierarchyComponent>()->Parent == root.GetID());
		assert(root.GetComponent<HierarchyComponent>()->Children.size() == 1);
		assert(scene.FindByName("GrandChild") == grandChild);
		assert(!scene.FindByName("Missing").IsValid());

		// Huỷ child kéo theo grandChild, và child bị gỡ khỏi danh sách con của root.
		scene.DestroyEntity(child);
		assert(!child.IsValid() && !grandChild.IsValid());
		assert(root.GetComponent<HierarchyComponent>()->Children.empty());
		assert(scene.GetEntities().size() == 2);

		// Không được huỷ Render Settings; Clear giữ lại nó và tuỳ chọn hiển thị.
		scene.GetRenderSettings().Wireframe = true;
		scene.DestroyEntity(scene.GetSettingsEntity());
		scene.Clear();
		assert(scene.GetEntities().empty());
		assert(AliveEntities() == base);
		assert(scene.GetSettingsEntity().IsValid());
		assert(scene.GetRenderSettings().Wireframe);
		(void)other;
	}
	// Huỷ Scene thì không còn entity nào.
	assert(AliveEntities() == 0);
	assert(ECS::ComponentManager::GetInstance().GetComponentCount<NameComponent>() == 0);
	std::cout << "  -> PASS\n";
}

void TestTransformLocalMatrix() {
	std::cout << "[RUN] Transform local matrix...\n";
	TransformComponent transform;
	transform.Position = { 1, 2, 3 };
	transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
	transform.Scale = glm::vec3(2);
	// Scale -> xoay -> dời: (1,0,0) -> (2,0,0) -> (0,0,-2) -> (1,2,1)
	glm::vec3 point = glm::vec3(transform.GetLocalMatrix() * glm::vec4(1, 0, 0, 1));
	assert(Vec3Equal(point, { 1, 2, 1 }));
	std::cout << "  -> PASS\n";
}

void TestLoadModelWithoutLights() {
	std::cout << "[RUN] Load model without lights...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	{
		Scene scene;
		Actor model = loader.LoadModel(scene, Asset("textured.gltf"));
		assert(model.IsValid());
		assert(model.GetComponent<NameComponent>()->Name == "textured");
		assert(model.GetComponent<ModelSourceComponent>()->Path == Asset("textured.gltf"));

		auto* renderer = model.GetComponent<MeshRendererComponent>();
		assert(renderer && renderer->Model && renderer->Visible);
		assert(renderer->Model->GetMeshes().size() == 1);

		// Bản B: entity giữ transform cục bộ của node (dời (2,3,4), scale 2), đỉnh để nguyên toạ độ cục bộ.
		auto* transform = model.GetComponent<TransformComponent>();
		assert(Vec3Equal(transform->Position, { 2, 3, 4 }) && Vec3Equal(transform->Scale, glm::vec3(2)));
		assert(!transform->Matrix);
		const auto& vertices = renderer->Model->GetMeshes().front().Vertices;
		assert(Vec3Equal(vertices[0].Position, { 0, 0, 0 }) && Vec3Equal(vertices[1].Position, { 1, 0, 0 }));

		// Bounding box giống kết quả test importer cũ (node dời (2,3,4), scale 2).
		auto* bounds = model.GetComponent<BoundsComponent>();
		assert(bounds->Box.isValid());
		assert(Vec3Equal(bounds->Box.getMin(), { 2, 3, 4 }));
		assert(Vec3Equal(bounds->Box.getMax(), { 4, 5, 4 }));

		// File không có đèn -> 1 đèn mặc định, là entity gốc riêng (không kế thừa transform của model).
		assert(scene.GetEntities().size() == 2);
		Actor light = scene.FindByName("Default Light");
		auto* lightComponent = light.GetComponent<LightComponent>();
		assert(lightComponent && !lightComponent->FromFile && lightComponent->Enabled);
		assert(lightComponent->Light.Type == Model::LightType::Directional);
		assert(Vec3Equal(lightComponent->Light.Direction, DEFAULT_LIGHT_DIRECTION));
		assert(light.GetComponent<HierarchyComponent>()->Parent == INVALID_ENTITY_ID);
		assert(scene.GetRoots().size() == 2);
	}
	assert(AliveEntities() == 0);
	std::cout << "  -> PASS\n";
}

void TestLoadModelWithLights() {
	std::cout << "[RUN] Load KHR_lights_punctual lights...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	Scene scene;
	Actor model = loader.LoadModel(scene, Asset("lights.gltf"));
	assert(model.IsValid());
	// Mỗi node 1 entity: gốc "lights" + Triangle, Sun, Bulb, Rig, Spot. Không có đèn mặc định.
	assert(scene.GetEntities().size() == 6);
	assert(!scene.FindByName("Default Light").IsValid());
	assert(model.GetComponent<HierarchyComponent>()->Children.size() == 4);
	Actor rig = scene.FindByName("Rig");
	Actor spotEntity = scene.FindByName("Spot");
	assert(spotEntity.GetComponent<HierarchyComponent>()->Parent == rig.GetID());
	assert(scene.FindByName("Triangle").HasComponent<MeshRendererComponent>());
	assert(!model.HasComponent<MeshRendererComponent>());

	// Đèn gắn trên entity của node, lưu theo toạ độ cục bộ của node.
	auto* sun = scene.FindByName("Sun").GetComponent<LightComponent>();
	assert(sun && sun->FromFile && sun->Light.Type == Model::LightType::Directional);
	assert(Vec3Equal(sun->Light.Direction, { 0, 0, -1 }));
	assert(Vec3Equal(sun->Light.Color, { 3, 3, 2.7f })); // color * intensity (3)
	auto* spot = spotEntity.GetComponent<LightComponent>();
	assert(spot && spot->Light.Type == Model::LightType::Spot);
	assert(Vec3Equal(spot->Light.Position, { 0, 0, 0 }));
	assert(std::fabs(spot->Light.InnerConeAngle - 0.2f) < 1e-4f);
	assert(std::fabs(spot->Light.OuterConeAngle - 0.5f) < 1e-4f);
	Actor bulbEntity = scene.FindByName("Bulb");
	assert(Vec3Equal(bulbEntity.GetComponent<TransformComponent>()->Position, { 1, 2, 3 }));

	// Sau TransformSystem + LightingSystem: toạ độ thế giới giống bản A.
	TransformSystem transforms;
	LightingSystem lighting;
	transforms.OnUpdate(0.0f);
	lighting.OnUpdate(0.0f);
	auto findLight = [&](Model::LightType type) -> const ShaderLight& {
		for (const auto& light : lighting.GetLights()) if (light.Type == type) return light;
		assert(false && "light not found");
		return lighting.GetLights().front();
	};
	// Sun: node xoay -90° quanh X -> ánh sáng chiếu thẳng xuống.
	assert(Vec3Equal(findLight(Model::LightType::Directional).Direction, { 0, -1, 0 }));
	// Bulb: point light tại node dời (1,2,3), màu (1,0.5,0.25) * 2.
	assert(Vec3Equal(findLight(Model::LightType::Point).Position, { 1, 2, 3 }));
	assert(Vec3Equal(findLight(Model::LightType::Point).Color, { 2, 1, 0.5f }));
	// Spot: node con (0,0,5) của node cha (0,10,0) -> vị trí thế giới (0,10,5), chiếu theo -Z.
	assert(Vec3Equal(findLight(Model::LightType::Spot).Position, { 0, 10, 5 }));
	assert(Vec3Equal(findLight(Model::LightType::Spot).Direction, { 0, 0, -1 }));

	// Bản A (RenderModel::GetLights) cho cùng kết quả thế giới.
	auto flat = importer.Import(Asset("lights.gltf").c_str());
	for (const auto& light : flat->GetLights()) {
		const ShaderLight& ecs = findLight(light.Type);
		assert(Vec3Equal(ecs.Position, light.Position));
		if (light.Type != Model::LightType::Point) assert(Vec3Equal(ecs.Direction, light.Direction)); // đèn point không có hướng
	}
	std::cout << "  -> PASS\n";
}

void TestLoadFailureKeepsScene() {
	std::cout << "[RUN] Failed load keeps the current scene...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	Scene scene;
	Actor model = loader.LoadModel(scene, Asset("textured.gltf"));
	const size_t count = scene.GetEntities().size();

	assert(!loader.LoadModel(scene, Asset("missing.glb")).IsValid());
	assert(!loader.LoadModel(scene, Asset("invalid.gltf")).IsValid());
	assert(!loader.LoadModel(scene, "").IsValid());
	assert(model.IsValid() && scene.GetEntities().size() == count);

	// Mở model khác thay hẳn model cũ, giữ tuỳ chọn hiển thị.
	scene.GetRenderSettings().Lighting = false;
	Actor next = loader.LoadModel(scene, Asset("lights.gltf"));
	// ECS tái dùng id đã huỷ, nên kiểm tra theo tên thay vì handle cũ `model`.
	assert(next.IsValid() && !scene.FindByName("textured").IsValid());
	assert(scene.GetEntities().size() == 6);
	assert(!scene.GetRenderSettings().Lighting);
	std::cout << "  -> PASS\n";
}

// Bounding box thế giới của bản B (theo cây node) phải trùng bản A (đỉnh đã áp transform) trên mọi file mẫu.
void TestNodeHierarchyMatchesFlattened() {
	std::cout << "[RUN] Node hierarchy (B) matches flattened import (A)...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	const std::string cube = (std::filesystem::path(TEST_ASSET_DIR) / "../../assets/models/Cube/Cube.fbx").string();
	for (const std::string& path : { Asset("textured.gltf"), Asset("embedded.glb"), Asset("shared-image.glb"),
		Asset("lights.gltf"), Asset("shear.gltf"), cube }) {
		auto flat = importer.Import(path.c_str());
		assert(flat);
		Model::AssimpBoundingBox expected;
		size_t expectedVertices = 0;
		for (const auto& mesh : flat->GetMeshes()) {
			expectedVertices += mesh.Vertices.size();
			for (const auto& vertex : mesh.Vertices) expected.expand(vertex.Position);
		}

		Scene scene;
		Actor root = loader.LoadModel(scene, path);
		assert(root.IsValid());
		const auto& box = root.GetComponent<BoundsComponent>()->Box;
		assert(Vec3Equal(box.getMin(), expected.getMin(), 1e-3f * std::max(1.0f, expected.radius())));
		assert(Vec3Equal(box.getMax(), expected.getMax(), 1e-3f * std::max(1.0f, expected.radius())));

		// Đỉnh nhân World của entity (sau TransformSystem) trùng đỉnh đã áp transform của bản A.
		TransformSystem transforms;
		transforms.OnUpdate(0.0f);
		Model::AssimpBoundingBox fromEntities;
		size_t vertices = 0;
		for (const Actor& entity : scene.GetEntities()) {
			auto* renderer = entity.GetComponent<MeshRendererComponent>();
			if (!renderer) continue;
			const glm::mat4& world = entity.GetComponent<TransformComponent>()->World;
			for (const auto& mesh : renderer->Model->GetMeshes()) {
				vertices += mesh.Vertices.size();
				for (const auto& vertex : mesh.Vertices) fromEntities.expand(glm::vec3(world * glm::vec4(vertex.Position, 1.0f)));
			}
		}
		assert(vertices == expectedVertices);
		assert(Vec3Equal(fromEntities.getMin(), expected.getMin(), 1e-3f * std::max(1.0f, expected.radius())));
		assert(Vec3Equal(fromEntities.getMax(), expected.getMax(), 1e-3f * std::max(1.0f, expected.radius())));
	}
	std::cout << "  -> PASS\n";
}

void TestShearedNodeKeepsMatrix() {
	std::cout << "[RUN] Node with shear keeps its raw matrix...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	Scene scene;
	Actor root = loader.LoadModel(scene, Asset("shear.gltf"));
	assert(root.IsValid());
	auto* transform = root.GetComponent<TransformComponent>();
	assert(transform->Matrix); // không tách được TRS
	assert(std::fabs((*transform->Matrix)[1][0] - 0.5f) < 1e-6f);
	assert(!scene.FindByName("Child").GetComponent<TransformComponent>()->Matrix);
	// Đỉnh (0,1,0) của tam giác ở node con: dời (1,0,0) rồi shear x += 0.5*y -> (1.5, 1, 0).
	TransformSystem transforms;
	transforms.OnUpdate(0.0f);
	const glm::mat4& world = scene.FindByName("Child").GetComponent<TransformComponent>()->World;
	assert(Vec3Equal(glm::vec3(world * glm::vec4(0, 1, 0, 1)), { 1.5f, 1, 0 }));

	// Hồi quy: đèn mặc định không được kế thừa shear của node gốc (từng làm lệch hướng sáng).
	LightingSystem lighting;
	lighting.OnUpdate(0.0f);
	assert(lighting.GetLights().size() == 1);
	assert(Vec3Equal(lighting.GetLights().front().Direction, DEFAULT_LIGHT_DIRECTION));
	std::cout << "  -> PASS\n";
}

void TestTooManyNodesFlattens() {
	std::cout << "[RUN] Too many nodes -> flattened into one entity...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer, 2); // lights.gltf có 6 node > 2
	Scene scene;
	Actor root = loader.LoadModel(scene, Asset("lights.gltf"));
	assert(root.IsValid() && root.HasComponent<MeshRendererComponent>());
	// Gốc giữ đèn đầu tiên, 2 đèn còn lại thành entity con; không có entity cho từng node.
	assert(scene.GetEntities().size() == 3);
	assert(!scene.FindByName("Rig").IsValid());

	TransformSystem transforms;
	LightingSystem lighting;
	transforms.OnUpdate(0.0f);
	lighting.OnUpdate(0.0f);
	assert(lighting.GetLights().size() == 3);
	bool foundSpot = false;
	for (const auto& light : lighting.GetLights()) {
		if (light.Type != Model::LightType::Spot) continue;
		foundSpot = true;
		assert(Vec3Equal(light.Position, { 0, 10, 5 })); // đèn đã ở toạ độ thế giới
	}
	assert(foundSpot);
	std::cout << "  -> PASS\n";
}

void TestViewStateJson() {
	std::cout << "[RUN] View state JSON...\n";
	ViewState state;
	state.ModelPath = "models/car.glb";
	state.HasCamera = true;
	state.CameraTarget = { 1, 2, 3 };
	state.CameraDistance = 4.5f;
	state.CameraAzimuth = 0.25f;
	state.CameraElevation = -0.5f;
	state.Wireframe = true;
	state.Lighting = false;
	state.HiddenNodes = { "Body/Wheel", "Body/Wheel#1" };
	state.DisabledLights = { "Sun" };

	std::string error;
	auto parsed = ViewStateFromJson(ViewStateToJson(state), error);
	assert(parsed && error.empty());
	assert(parsed->ModelPath == state.ModelPath && parsed->HasCamera);
	assert(Vec3Equal(parsed->CameraTarget, state.CameraTarget));
	assert(parsed->CameraDistance == 4.5f && parsed->CameraAzimuth == 0.25f && parsed->CameraElevation == -0.5f);
	assert(parsed->Wireframe && parsed->Textures && !parsed->Lighting);
	assert(parsed->HiddenNodes == state.HiddenNodes && parsed->DisabledLights == state.DisabledLights);

	// Chỉ có version + model: các mục khác lấy mặc định.
	auto minimal = ViewStateFromJson(R"({"version": 1, "model": "a.gltf"})", error);
	assert(minimal && !minimal->HasCamera && !minimal->Wireframe && minimal->Textures && minimal->HiddenNodes.empty());

	// File hỏng / sai: báo lỗi, không crash.
	for (const char* bad : {
		"not json",
		"[1, 2]",
		R"({"model": "a.gltf"})",                               // thiếu version
		R"({"version": 2, "model": "a.gltf"})",                 // version không hỗ trợ
		R"({"version": 1})",                                    // thiếu model
		R"({"version": 1, "model": ""})",
		R"({"version": 1, "model": 5})",                        // sai kiểu
		R"({"version": 1, "model": "a", "camera": {"target": [0, 0], "distance": 1, "azimuth": 0, "elevation": 0}})",
		R"({"version": 1, "model": "a", "camera": {"target": [0, 0, 0], "distance": 0, "azimuth": 0, "elevation": 0}})",
		R"({"version": 1, "model": "a", "render": {"wireframe": "yes"}})",
		R"({"version": 1, "model": "a", "hiddenNodes": "Body"})" }) {
		error.clear();
		assert(!ViewStateFromJson(bad, error) && !error.empty());
	}
	std::cout << "  -> PASS\n";
}

void TestNodePaths() {
	std::cout << "[RUN] Node paths...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	{
		Scene scene;
		Actor root = loader.LoadModel(scene, Asset("lights.gltf"));
		assert(GetNodePath(root, root) == std::string());
		assert(GetNodePath(scene.FindByName("Spot"), root) == std::string("Rig/Spot"));
		assert(GetNodePath(scene.FindByName("Triangle"), root) == std::string("Triangle"));
		assert(FindByNodePath(root, "Rig/Spot") == scene.FindByName("Spot"));
		assert(FindByNodePath(root, "") == root);
		assert(!FindByNodePath(root, "Rig/Missing").IsValid());
		for (const Actor& entity : scene.GetEntities()) {
			auto path = GetNodePath(entity, root);
			assert(path && FindByNodePath(root, *path) == entity);
		}
		// Entity ngoài cây của model (vd camera) không có đường dẫn.
		assert(!GetNodePath(scene.CreateEntity("Camera"), root));

		// Anh em trùng tên được đánh số theo thứ tự.
		Actor parent = scene.CreateEntity("Parent", root);
		Actor first = scene.CreateEntity("Wheel", parent);
		Actor second = scene.CreateEntity("Wheel", parent);
		assert(GetNodePath(first, root) == std::string("Parent/Wheel"));
		assert(GetNodePath(second, root) == std::string("Parent/Wheel#1"));
		assert(FindByNodePath(root, "Parent/Wheel#1") == second);
	}
	std::cout << "  -> PASS\n";
}

std::string ReadFile(const std::filesystem::path& path) {
	std::ifstream stream(path, std::ios::binary);
	std::stringstream content;
	content << stream.rdbuf();
	return content.str();
}

void TestViewStateRoundTrip() {
	std::cout << "[RUN] Save and load a view...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	const auto directory = std::filesystem::temp_directory_path() / "inputprocessor-test-view";
	std::filesystem::create_directories(directory);
	const auto viewFile = directory / "lights.view.json";
	const std::string modelBefore = ReadFile(Asset("lights.gltf"));

	CameraSystem cameras(nullptr);
	cameras.SetViewportSize(640, 480);
	ViewState saved;
	{
		Scene scene;
		Actor root = loader.LoadModel(scene, Asset("lights.gltf"));
		auto* camera = cameras.CreateCamera(scene, &root.GetComponent<BoundsComponent>()->Box).GetComponent<CameraComponent>();
		camera->Camera.SetAzimuth(0.7f);
		camera->Camera.SetElevation(0.3f);
		camera->Camera.SetDistance(camera->Camera.GetDistance() * 1.5f);
		scene.FindByName("Triangle").GetComponent<MeshRendererComponent>()->Visible = false;
		FindByNodePath(root, "Rig/Spot").GetComponent<LightComponent>()->Enabled = false;
		scene.GetRenderSettings().Wireframe = true;
		scene.GetRenderSettings().Textures = false;

		saved = CaptureViewState(scene, root, camera, viewFile);
		std::string error;
		assert(SaveViewState(saved, viewFile, error));
		assert(saved.HiddenNodes == std::vector<std::string>{ "Triangle" });
		assert(saved.DisabledLights == std::vector<std::string>{ "Rig/Spot" });
		// Đường dẫn model ghi tương đối với thư mục file trạng thái (nếu được), và trỏ đúng file.
		assert(std::filesystem::equivalent(ResolveModelPath(saved, viewFile), Asset("lights.gltf")));
	}
	// Lưu trạng thái không bao giờ sửa file model.
	assert(ReadFile(Asset("lights.gltf")) == modelBefore);

	{
		std::string error;
		auto loaded = LoadViewState(viewFile, error);
		assert(loaded);
		Scene scene;
		Actor root = loader.LoadModel(scene, ResolveModelPath(*loaded, viewFile).string());
		assert(root.IsValid());
		auto* camera = cameras.CreateCamera(scene, &root.GetComponent<BoundsComponent>()->Box).GetComponent<CameraComponent>();
		assert(ApplyViewState(*loaded, scene, root, camera) == 0);

		assert(!scene.FindByName("Triangle").GetComponent<MeshRendererComponent>()->Visible);
		assert(!scene.FindByName("Spot").GetComponent<LightComponent>()->Enabled);
		assert(scene.FindByName("Bulb").GetComponent<LightComponent>()->Enabled);
		assert(scene.GetRenderSettings().Wireframe && !scene.GetRenderSettings().Textures && scene.GetRenderSettings().Lighting);
		assert(Vec3Equal(camera->Camera.GetTarget(), saved.CameraTarget));
		assert(std::fabs(camera->Camera.GetDistance() - saved.CameraDistance) < 1e-4f);
		assert(std::fabs(camera->Camera.GetAzimuth() - 0.7f) < 1e-4f);
		assert(std::fabs(camera->Camera.GetElevation() - 0.3f) < 1e-4f);

		// Node không còn trong model (vd model đã đổi) -> bỏ qua và đếm, không crash.
		ViewState stale = *loaded;
		stale.HiddenNodes.push_back("Gone");
		stale.DisabledLights.push_back("Triangle"); // node có nhưng không phải đèn
		assert(ApplyViewState(stale, scene, root, camera) == 2);
	}
	assert(!LoadViewState(directory / "missing.view.json", *std::make_unique<std::string>()));
	std::filesystem::remove_all(directory);
	std::cout << "  -> PASS\n";
}

void TestTransformSystem() {
	std::cout << "[RUN] TransformSystem...\n";
	{
		Scene scene;
		Actor root = scene.CreateEntity("Root");
		Actor child = scene.CreateEntity("Child", root);
		Actor grandChild = scene.CreateEntity("GrandChild", child);
		root.GetComponent<TransformComponent>()->Position = { 1, 0, 0 };
		root.GetComponent<TransformComponent>()->Scale = glm::vec3(2);
		child.GetComponent<TransformComponent>()->Position = { 0, 1, 0 };
		grandChild.GetComponent<TransformComponent>()->Position = { 0, 0, 1 };

		TransformSystem system;
		system.OnUpdate(0.0f);
		auto worldOf = [](Actor actor) { return glm::vec3(actor.GetComponent<TransformComponent>()->World[3]); };
		// Scale 2 của gốc áp lên mọi cấp con: (1,0,0) + 2*(0,1,0) + 2*(0,0,1)
		assert(Vec3Equal(worldOf(root), { 1, 0, 0 }));
		assert(Vec3Equal(worldOf(child), { 1, 2, 0 }));
		assert(Vec3Equal(worldOf(grandChild), { 1, 2, 2 }));
		assert(!grandChild.GetComponent<TransformComponent>()->Dirty);

		// Không Dirty -> không tính lại (giữ nguyên giá trị đã ghi đè).
		grandChild.GetComponent<TransformComponent>()->World = glm::mat4(0.0f);
		system.OnUpdate(0.0f);
		assert(grandChild.GetComponent<TransformComponent>()->World == glm::mat4(0.0f));

		// Cha đổi -> cả nhánh con được tính lại.
		root.GetComponent<TransformComponent>()->Position = { 5, 0, 0 };
		root.GetComponent<TransformComponent>()->Dirty = true;
		system.OnUpdate(0.0f);
		assert(Vec3Equal(worldOf(grandChild), { 5, 2, 2 }));
	}
	std::cout << "  -> PASS\n";
}

void TestLightingSystem() {
	std::cout << "[RUN] LightingSystem...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	{
		Scene scene;
		loader.LoadModel(scene, Asset("lights.gltf"));
		LightingSystem system;
		system.OnUpdate(0.0f);
		assert(system.GetLights().size() == 3 && system.GetDroppedCount() == 0);

		const ShaderLight* spot = nullptr;
		for (const auto& light : system.GetLights()) if (light.Type == Model::LightType::Spot) spot = &light;
		assert(spot && std::fabs(spot->CosInnerCone - std::cos(0.2f)) < 1e-5f && std::fabs(spot->CosOuterCone - std::cos(0.5f)) < 1e-5f);

		// Đèn tắt không được gửi cho shader.
		scene.FindByName("Bulb").GetComponent<LightComponent>()->Enabled = false;
		system.OnUpdate(0.0f);
		assert(system.GetLights().size() == 2);

		// Quá giới hạn shader -> cắt bớt và đếm số bị bỏ.
		for (int i = 0; i < 10; ++i) scene.CreateEntity("Extra").AddComponent<LightComponent>();
		system.OnUpdate(0.0f);
		assert(system.GetLights().size() == MAX_SHADER_LIGHTS);
		assert(system.GetDroppedCount() == 12 - MAX_SHADER_LIGHTS);
	}
	std::cout << "  -> PASS\n";
}

void TestCameraSystem() {
	std::cout << "[RUN] CameraSystem...\n";
	TestResources resources;
	Model::AssimpModelImporter importer(&resources);
	SceneLoader loader(importer);
	{
		Scene scene;
		Actor model = loader.LoadModel(scene, Asset("textured.gltf"));
		const auto& box = model.GetComponent<BoundsComponent>()->Box;

		CameraSystem noInput(nullptr);
		noInput.SetViewportSize(800, 600);
		noInput.SetViewportSize(0, 0); // thu nhỏ cửa sổ về 0: bỏ qua, không chia cho 0
		assert(noInput.GetViewportWidth() == 800 && noInput.GetViewportHeight() == 600);

		Actor cameraEntity = noInput.CreateCamera(scene, &box);
		auto* camera = cameraEntity.GetComponent<CameraComponent>();
		assert(camera && CameraSystem::GetActiveCamera() == camera);
		assert(Vec3Equal(camera->Camera.GetTarget(), box.getCenter()));
		assert(std::fabs(camera->Camera.GetAspect() - 800.0f / 600.0f) < 1e-4f);
		// Mọi góc của model nằm trong khung hình.
		const glm::mat4 viewProjection = camera->Camera.GetProjectionMatrix() * camera->Camera.GetViewMatrix();
		const glm::mat4 world = model.GetComponent<TransformComponent>()->GetLocalMatrix(); // node gốc: world = local
		for (const auto& vertex : model.GetComponent<MeshRendererComponent>()->Model->GetMeshes().front().Vertices) {
			glm::vec4 clip = viewProjection * world * glm::vec4(vertex.Position, 1.0f);
			glm::vec3 ndc = glm::vec3(clip) / clip.w;
			assert(std::fabs(ndc.x) < 1 && std::fabs(ndc.y) < 1 && std::fabs(ndc.z) < 1);
		}

		// Resize áp cho camera đã có.
		noInput.SetViewportSize(400, 800);
		assert(std::fabs(camera->Camera.GetAspect() - 0.5f) < 1e-4f);

		// Không có input -> không tạo controller, không crash.
		noInput.OnUpdate(0.016f);
		assert(!camera->Controller);

		// Có input -> controller được tạo cho camera Active và xử lý kéo chuột.
		CoreEngine::WxMouseInput mouse;
		CoreEngine::WxKeyboardInput keyboard;
		CoreEngine::InputState input{ &keyboard, &mouse };
		CameraSystem withInput(&input);
		withInput.OnUpdate(0.016f);
		assert(camera->Controller);
		const glm::vec3 before = camera->Camera.GetPosition();
		mouse.NotifyButtonDown(CoreEngine::MouseButton::ButtonLeft);
		mouse.SetDelta(10.0f, 0.0f);
		withInput.OnUpdate(0.016f);
		withInput.OnUpdate(0.016f);
		assert(!Vec3Equal(camera->Camera.GetPosition(), before));

		// Bounds rỗng (model không có đỉnh) -> giữ camera mặc định, không crash.
		Model::AssimpBoundingBox empty;
		Actor fallback = noInput.CreateCamera(scene, &empty);
		assert(fallback.IsValid());

		camera->Active = false;
		assert(CameraSystem::GetActiveCamera() == fallback.GetComponent<CameraComponent>());
		fallback.GetComponent<CameraComponent>()->Active = false;
		assert(CameraSystem::GetActiveCamera() == nullptr);
	}
	std::cout << "  -> PASS\n";
}

void TestRenderSystem() {
	std::cout << "[RUN] RenderSystem with a fake renderer...\n";
	FakeRenderer renderer;
	Model::AssimpModelImporter importer(&renderer.Resources);
	SceneLoader loader(importer);
	CoreEngine::ECS::Shutdown();
	AddViewerSystems(nullptr, &renderer);
	auto& systems = ECS::SystemManager::GetInstance();
	assert(!systems.HasDependencyCycle());
	auto* render = systems.GetSystem<RenderSystem>();
	assert(render && render->IsReady());
	{
		Scene scene;
		ECS::Update(0.016f); // chưa có camera: chỉ xoá màn hình
		assert(renderer.Command.Clears == 1 && render->GetLastDrawCount() == 0);

		Actor model = loader.LoadModel(scene, Asset("textured.gltf"));
		systems.GetSystem<CameraSystem>()->CreateCamera(scene, &model.GetComponent<BoundsComponent>()->Box);
		ECS::Update(0.016f);
		assert(render->GetLastDrawCount() == 1);
		assert(renderer.Command.DrawnIndexCounts == std::vector<unsigned int>{ 3 });
		assert(renderer.Resources.VertexBuffersCreated == 1);
		FakeShader& shader = renderer.Resources.Shader;
		assert(shader.Get("uEnableTextures") == 1 && shader.Get("uEnableLighting") == 1);
		assert(shader.Get("uLightCount") == 1 && shader.Get("uLights[0].type") == 0); // đèn mặc định
		// uModel = world của node (dời (2,3,4), scale 2).
		const glm::mat4 nodeWorld = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(2, 3, 4)), glm::vec3(2));
		const auto& uploaded = shader.Uniforms["uModel"];
		for (int i = 0; i < 16; ++i) assert(std::fabs(uploaded[i] - glm::value_ptr(nodeWorld)[i]) < 1e-5f);
		assert(renderer.Command.PolygonModes.back() == CoreEngine::PolygonMode::Fill);

		// Bật wireframe, tắt texture + lighting.
		auto& settings = scene.GetRenderSettings();
		settings.Wireframe = true;
		settings.Textures = false;
		settings.Lighting = false;
		renderer.Command.PolygonModes.clear();
		ECS::Update(0.016f);
		assert((renderer.Command.PolygonModes == std::vector{ CoreEngine::PolygonMode::Line, CoreEngine::PolygonMode::Fill }));
		assert(shader.Get("uEnableTextures") == 0 && shader.Get("uEnableLighting") == 0);
		assert(renderer.Resources.VertexBuffersCreated == 1); // lazy-upload: dùng lại buffer frame trước

		// Ẩn model -> không vẽ.
		model.GetComponent<MeshRendererComponent>()->Visible = false;
		ECS::Update(0.016f);
		assert(render->GetLastDrawCount() == 0);
	}
	CoreEngine::ECS::Shutdown();
	std::cout << "  -> PASS\n";
}

int main() {
	TestHierarchyAndDestroy();
	TestTransformLocalMatrix();
	TestLoadModelWithoutLights();
	TestLoadModelWithLights();
	TestLoadFailureKeepsScene();
	TestNodeHierarchyMatchesFlattened();
	TestShearedNodeKeepsMatrix();
	TestTooManyNodesFlattens();
	TestViewStateJson();
	TestNodePaths();
	TestViewStateRoundTrip();
	TestTransformSystem();
	TestLightingSystem();
	TestCameraSystem();
	TestRenderSystem();
	CoreEngine::ECS::Shutdown();
	std::cout << "All scene tests passed.\n";
	return 0;
}
