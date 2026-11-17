#pragma once
#include <ECS/ISystem.hpp>
#include <Scene/Components.hpp>

namespace Domain::Scene {
	// Tính TransformComponent::World = World(cha) * Local, duyệt cây từ entity gốc.
	// Chỉ tính lại nhánh có Dirty (hoặc có cha vừa đổi), rồi xoá cờ Dirty.
	class TransformSystem : public CoreEngine::ECS::ISystem {
	public:
		TransformSystem() : ISystem(CoreEngine::ECS::ESystemPriority::High) {}

		void OnUpdate(float deltaTime) override;

	private:
		void UpdateBranch(EntityID entity, const glm::mat4& parentWorld, bool parentChanged);
	};
}
