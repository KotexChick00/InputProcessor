// Giữ assert hoạt động cả khi build Release.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <Scene/Scene.hpp>
#include <Scene/SceneLoader.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Renderer/Resource/IResourceManager.hpp>

#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>

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

		// Bản A: đỉnh đã áp node transform, entity giữ transform đơn vị.
		auto* transform = model.GetComponent<TransformComponent>();
		assert(Vec3Equal(transform->Position, glm::vec3(0)) && Vec3Equal(transform->Scale, glm::vec3(1)));

		// Bounding box giống kết quả test importer cũ (node dời (2,3,4), scale 2).
		auto* bounds = model.GetComponent<BoundsComponent>();
		assert(bounds->Box.isValid());
		assert(Vec3Equal(bounds->Box.getMin(), { 2, 3, 4 }));
		assert(Vec3Equal(bounds->Box.getMax(), { 4, 5, 4 }));

		// File không có đèn -> 1 đèn mặc định, là con của model.
		assert(scene.GetEntities().size() == 2);
		Actor light = scene.FindByName("Default Light");
		auto* lightComponent = light.GetComponent<LightComponent>();
		assert(lightComponent && !lightComponent->FromFile && lightComponent->Enabled);
		assert(lightComponent->Light.Type == Model::LightType::Directional);
		assert(Vec3Equal(lightComponent->Light.Direction, DEFAULT_LIGHT_DIRECTION));
		assert(light.GetComponent<HierarchyComponent>()->Parent == model.GetID());
		assert(scene.GetRoots().size() == 1);
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
	assert(scene.GetEntities().size() == 4); // model + 3 đèn, không có đèn mặc định
	assert(!scene.FindByName("Default Light").IsValid());

	// Sun: node xoay -90° quanh X -> ánh sáng chiếu thẳng xuống. Màu = color * intensity (3).
	auto* sun = scene.FindByName("Sun").GetComponent<LightComponent>();
	assert(sun && sun->FromFile && sun->Light.Type == Model::LightType::Directional);
	assert(Vec3Equal(sun->Light.Direction, { 0, -1, 0 }));
	assert(Vec3Equal(sun->Light.Color, { 3, 3, 2.7f }));

	// Bulb: point light tại node dời (1,2,3), màu (1,0.5,0.25) * 2.
	Actor bulbEntity = scene.FindByName("Bulb");
	auto* bulb = bulbEntity.GetComponent<LightComponent>();
	assert(bulb && bulb->Light.Type == Model::LightType::Point);
	assert(Vec3Equal(bulb->Light.Position, { 1, 2, 3 }));
	assert(Vec3Equal(bulb->Light.Color, { 2, 1, 0.5f }));
	assert(Vec3Equal(bulbEntity.GetComponent<TransformComponent>()->Position, { 1, 2, 3 }));

	// Spot: node con (0,0,5) của node cha (0,10,0) -> vị trí thế giới (0,10,5).
	auto* spot = scene.FindByName("Spot").GetComponent<LightComponent>();
	assert(spot && spot->Light.Type == Model::LightType::Spot);
	assert(Vec3Equal(spot->Light.Position, { 0, 10, 5 }));
	assert(Vec3Equal(spot->Light.Direction, { 0, 0, -1 }));
	assert(std::fabs(spot->Light.InnerConeAngle - 0.2f) < 1e-4f);
	assert(std::fabs(spot->Light.OuterConeAngle - 0.5f) < 1e-4f);
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
	assert(scene.GetEntities().size() == 4);
	assert(!scene.GetRenderSettings().Lighting);
	std::cout << "  -> PASS\n";
}

int main() {
	TestHierarchyAndDestroy();
	TestTransformLocalMatrix();
	TestLoadModelWithoutLights();
	TestLoadModelWithLights();
	TestLoadFailureKeepsScene();
	CoreEngine::ECS::Shutdown();
	std::cout << "All scene tests passed.\n";
	return 0;
}
