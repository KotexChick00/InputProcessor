#include <Scene/Systems/TransformSystem.hpp>
#include <ECS/ComponentManager.hpp>

namespace Domain::Scene {
	void TransformSystem::OnUpdate(float) {
		auto& components = CoreEngine::ECS::ComponentManager::GetInstance();

		for (TransformComponent& transform : components.GetAllComponents<TransformComponent>()) {
			auto* hierarchy = components.GetComponentByEntityID<HierarchyComponent>(transform.GetOwner());
			if (hierarchy && hierarchy->Parent != INVALID_ENTITY_ID) continue; // nhánh con được tính từ gốc
			UpdateBranch(transform.GetOwner(), glm::mat4(1.0f), false);
		}
	}

	void TransformSystem::UpdateBranch(EntityID entity, const glm::mat4& parentWorld, bool parentChanged) {
		auto& components = CoreEngine::ECS::ComponentManager::GetInstance();
		auto* transform = components.GetComponentByEntityID<TransformComponent>(entity);
		if (!transform) return;

		const bool changed = parentChanged || transform->Dirty;
		if (changed) {
			transform->World = parentWorld * transform->GetLocalMatrix();
			transform->Dirty = false;
		}

		if (auto* hierarchy = components.GetComponentByEntityID<HierarchyComponent>(entity)) {
			for (EntityID child : hierarchy->Children) UpdateBranch(child, transform->World, changed);
		}
	}
}
