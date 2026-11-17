#include <Scene/SceneLoader.hpp>
#include <Logger/Logger.hpp>

#include <exception>
#include <filesystem>

namespace Domain::Scene {
	namespace {
		bool AddLight(Scene& scene, Actor parent, const Model::ImportedLight& light, bool fromFile) {
			Actor entity = scene.CreateEntity(light.Name.empty() ? "Light" : light.Name, parent);
			if (!entity.IsValid()) return false;

			entity.GetComponent<TransformComponent>()->Position = light.Position;
			auto* component = entity.AddComponent<LightComponent>();
			if (!component) {
				scene.DestroyEntity(entity);
				return false;
			}
			component->Light = light;
			component->FromFile = fromFile;
			return true;
		}
	}

	Actor SceneLoader::LoadModel(Scene& scene, const std::string& path) {
		std::unique_ptr<Model::RenderModel> model;
		try {
			model = mImporter.Import(path.c_str());
		} catch (const std::exception& error) {
			IP_ENGINE_ERROR("SceneLoader::LoadModel: failed to import '{}': {}", path, error.what());
			return {};
		}
		if (!model) {
			IP_ENGINE_ERROR("SceneLoader::LoadModel: failed to import '{}'", path);
			return {};
		}

		// Chỉ xoá scene cũ khi model mới đã đọc thành công.
		scene.Clear();

		Actor entity = scene.CreateEntity(std::filesystem::path(path).stem().string());
		if (!entity.IsValid()) return {};

		// Bounding box tính trên đỉnh đã áp node transform.
		auto* bounds = entity.AddComponent<BoundsComponent>();
		if (bounds) {
			for (const auto& mesh : model->GetMeshes()) {
				for (const auto& vertex : mesh.Vertices) bounds->Box.expand(vertex.Position);
			}
		}

		std::vector<Model::ImportedLight> lights = model->GetLights();
		if (!bounds
			|| !entity.AddComponent<ModelSourceComponent>(path)
			|| !entity.AddComponent<MeshRendererComponent>(std::move(model))) {
			IP_ENGINE_ERROR("SceneLoader::LoadModel: out of component memory for '{}'", path);
			scene.DestroyEntity(entity);
			return {};
		}

		bool lightsOk = true;
		for (const auto& light : lights) lightsOk = AddLight(scene, entity, light, true) && lightsOk;
		if (lights.empty()) {
			Model::ImportedLight light;
			light.Name = "Default Light";
			light.Type = Model::LightType::Directional;
			light.Direction = DEFAULT_LIGHT_DIRECTION;
			lightsOk = AddLight(scene, entity, light, false);
		}
		if (!lightsOk) IP_ENGINE_WARN("SceneLoader::LoadModel: some lights of '{}' couldn't be added", path);

		IP_ENGINE_DEBUG("SceneLoader::LoadModel: loaded '{}' ({} lights from file)", path, lights.size());
		return entity;
	}
}
