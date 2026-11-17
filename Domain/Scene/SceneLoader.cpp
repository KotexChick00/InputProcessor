#include <Scene/SceneLoader.hpp>
#include <Logger/Logger.hpp>

#include <exception>
#include <filesystem>

namespace Domain::Scene {
	namespace {
		bool AddLight(Scene& scene, Actor owner, const Model::ImportedLight& light, bool fromFile) {
			// Mỗi entity chỉ có 1 LightComponent: đèn thứ hai trở đi của cùng node thành entity con.
			Actor entity = owner;
			if (!entity.IsValid() || entity.HasComponent<LightComponent>()) {
				entity = scene.CreateEntity(light.Name.empty() ? "Light" : light.Name, owner);
				if (!entity.IsValid()) return false;
			}
			auto* component = entity.AddComponent<LightComponent>();
			if (!component) return false;
			component->Light = light;
			component->FromFile = fromFile;
			return true;
		}

		void ApplyLocalTransform(TransformComponent& transform, const Model::ImportedNode& node) {
			transform.Position = node.Position;
			transform.Rotation = node.Rotation;
			transform.Scale = node.Scale;
			if (!node.IsTrs) transform.Matrix = node.Local;
			transform.Dirty = true;
		}
	}

	Actor SceneLoader::LoadModel(Scene& scene, const std::string& path) {
		std::unique_ptr<Model::ImportedScene> imported;
		try {
			imported = mImporter.ImportScene(path.c_str(), mMaxNodeEntities);
		} catch (const std::exception& error) {
			IP_ENGINE_ERROR("SceneLoader::LoadModel: failed to import '{}': {}", path, error.what());
			return {};
		}
		if (!imported || imported->Nodes.empty()) {
			IP_ENGINE_ERROR("SceneLoader::LoadModel: failed to import '{}'", path);
			return {};
		}

		// Chỉ xoá scene cũ khi model mới đã đọc thành công.
		scene.Clear();

		const auto& nodes = imported->Nodes;
		std::vector<Actor> entities(nodes.size());
		std::vector<glm::mat4> worlds(nodes.size(), glm::mat4(1.0f));
		Model::AssimpBoundingBox bounds;
		bool hasLights = false;
		bool ok = true;

		for (size_t i = 0; i < nodes.size() && ok; ++i) {
			auto& node = imported->Nodes[i];
			const bool isRoot = node.Parent < 0;
			Actor parent = isRoot ? Actor() : entities[node.Parent];
			std::string name = isRoot ? std::filesystem::path(path).stem().string()
				: (node.Name.empty() ? "Node" : node.Name);

			Actor entity = scene.CreateEntity(std::move(name), parent);
			if (!entity.IsValid()) { ok = false; break; }
			entities[i] = entity;
			ApplyLocalTransform(*entity.GetComponent<TransformComponent>(), node);
			worlds[i] = (isRoot ? glm::mat4(1.0f) : worlds[node.Parent]) * entity.GetComponent<TransformComponent>()->GetLocalMatrix();

			if (node.Model) {
				// Bounding box theo toạ độ thế giới (đỉnh cục bộ * world của node).
				for (const auto& mesh : node.Model->GetMeshes()) {
					for (const auto& vertex : mesh.Vertices) bounds.expand(glm::vec3(worlds[i] * glm::vec4(vertex.Position, 1.0f)));
				}
				ok = entity.AddComponent<MeshRendererComponent>(std::move(node.Model)) != nullptr;
			}
			for (const auto& light : node.Lights) {
				ok = ok && AddLight(scene, entity, light, true);
				hasLights = true;
			}
		}

		Actor root = entities.empty() ? Actor() : entities.front();
		if (ok && root.IsValid()) {
			auto* boundsComponent = root.AddComponent<BoundsComponent>();
			ok = boundsComponent && root.AddComponent<ModelSourceComponent>(path);
			if (boundsComponent) boundsComponent->Box = bounds;
		}
		if (ok && !hasLights) {
			Model::ImportedLight light;
			light.Name = "Default Light";
			light.Type = Model::LightType::Directional;
			light.Direction = DEFAULT_LIGHT_DIRECTION;
			// Đèn của viewer, không thuộc model: entity gốc riêng để không kế thừa transform của model
			// (node gốc có shear/xoay sẽ làm lệch hướng sáng).
			Actor lightEntity = scene.CreateEntity("Default Light");
			ok = lightEntity.IsValid() && AddLight(scene, lightEntity, light, false);
		}

		if (!ok) {
			IP_ENGINE_ERROR("SceneLoader::LoadModel: out of component memory for '{}'", path);
			scene.Clear();
			return {};
		}

		IP_ENGINE_DEBUG("SceneLoader::LoadModel: loaded '{}' ({} nodes{})", path, nodes.size(), imported->Flattened ? ", flattened" : "");
		return root;
	}
}
