#include <Scene/Scene.hpp>
#include <Logger/Logger.hpp>

#include <algorithm>
#include <stdexcept>

namespace Domain::Scene {
	Scene::Scene() {
		mSettings = CoreEngine::ECS::CreateActor();
		if (!mSettings.AddComponent<NameComponent>("Render Settings")
			|| !mSettings.AddComponent<RenderSettingsComponent>()) {
			mSettings.Destroy();
			throw std::runtime_error("Scene: can't allocate the render settings entity");
		}
	}

	Scene::~Scene() {
		Clear();
		if (mSettings.IsValid()) mSettings.Destroy();
	}

	Actor Scene::CreateEntity(std::string name, Actor parent) {
		Actor entity = CoreEngine::ECS::CreateActor();
		auto* hierarchy = entity.AddComponent<HierarchyComponent>();
		if (!entity.AddComponent<NameComponent>(std::move(name))
			|| !entity.AddComponent<TransformComponent>()
			|| !hierarchy) {
			IP_ENGINE_ERROR("Scene::CreateEntity: out of component memory");
			entity.Destroy();
			return {};
		}

		if (parent.IsValid()) {
			if (auto* parentHierarchy = parent.GetComponent<HierarchyComponent>()) {
				hierarchy->Parent = parent.GetID();
				parentHierarchy->Children.push_back(entity.GetID());
			} else {
				IP_ENGINE_WARN("Scene::CreateEntity: parent {} has no hierarchy, entity becomes a root", parent.GetID());
			}
		}

		mEntities.push_back(entity);
		return entity;
	}

	void Scene::DestroyEntity(Actor entity) {
		if (!entity.IsValid() || entity == mSettings) return;

		// Gỡ khỏi danh sách con của entity cha.
		if (auto* hierarchy = entity.GetComponent<HierarchyComponent>()) {
			Actor parent = CoreEngine::ECS::GetActor(hierarchy->Parent);
			if (auto* parentHierarchy = parent.IsValid() ? parent.GetComponent<HierarchyComponent>() : nullptr) {
				std::erase(parentHierarchy->Children, entity.GetID());
			}
		}
		DestroyRecursive(entity);
	}

	void Scene::DestroyRecursive(Actor entity) {
		if (auto* hierarchy = entity.GetComponent<HierarchyComponent>()) {
			// Copy vì con sẽ bị huỷ trong lúc duyệt.
			std::vector<EntityID> children = hierarchy->Children;
			for (EntityID child : children) {
				Actor childActor = CoreEngine::ECS::GetActor(child);
				if (childActor.IsValid()) DestroyRecursive(childActor);
			}
		}
		std::erase(mEntities, entity);
		entity.Destroy();
	}

	void Scene::Clear() {
		// Huỷ từ cuối lên để con (tạo sau cha) đi trước; entity đã bị huỷ theo cha thì bỏ qua.
		while (!mEntities.empty()) {
			Actor entity = mEntities.back();
			if (entity.IsValid()) {
				DestroyEntity(entity); // gỡ khỏi cha để không còn id treo trong Children
			} else {
				mEntities.pop_back();
			}
		}
	}

	std::vector<Actor> Scene::GetRoots() const {
		std::vector<Actor> roots;
		for (const Actor& entity : mEntities) {
			auto* hierarchy = entity.GetComponent<HierarchyComponent>();
			if (hierarchy && hierarchy->Parent == INVALID_ENTITY_ID) roots.push_back(entity);
		}
		return roots;
	}

	Actor Scene::FindByName(std::string_view name) const {
		for (const Actor& entity : mEntities) {
			auto* component = entity.GetComponent<NameComponent>();
			if (component && component->Name == name) return entity;
		}
		return {};
	}

	RenderSettingsComponent& Scene::GetRenderSettings() const {
		return *mSettings.GetComponent<RenderSettingsComponent>();
	}
}
