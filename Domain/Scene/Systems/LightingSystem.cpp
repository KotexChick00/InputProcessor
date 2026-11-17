#include <Scene/Systems/LightingSystem.hpp>
#include <ECS/ComponentManager.hpp>
#include <Logger/Logger.hpp>

#include <cmath>

namespace Domain::Scene {
	void LightingSystem::OnUpdate(float) {
		mLights.clear();
		mDropped = 0;

		auto& components = CoreEngine::ECS::ComponentManager::GetInstance();
		for (const LightComponent& component : components.GetAllComponents<LightComponent>()) {
			if (!component.Enabled) continue;
			if (mLights.size() >= MAX_SHADER_LIGHTS) {
				++mDropped;
				continue;
			}

			// Đèn lưu theo toạ độ cục bộ của entity -> đổi sang toạ độ thế giới.
			const Model::ImportedLight& light = component.Light;
			auto* transform = components.GetComponentByEntityID<TransformComponent>(component.GetOwner());
			const glm::mat4 world = transform ? transform->World : glm::mat4(1.0f);
			const glm::vec3 direction = glm::mat3(world) * light.Direction;

			ShaderLight result;
			result.Type = light.Type;
			result.Position = glm::vec3(world * glm::vec4(light.Position, 1.0f));
			result.Direction = glm::length(direction) > 0.0f ? glm::normalize(direction) : glm::vec3(0, 0, -1);
			result.Color = light.Color;
			if (light.Type == Model::LightType::Spot) {
				result.CosInnerCone = std::cos(light.InnerConeAngle);
				result.CosOuterCone = std::cos(light.OuterConeAngle);
			}
			mLights.push_back(result);
		}

		if (mDropped > 0 && !mWarnedAboutDropped) {
			IP_ENGINE_WARN("LightingSystem: {} lights ignored, the shader supports at most {}", mDropped, MAX_SHADER_LIGHTS);
			mWarnedAboutDropped = true;
		}
	}
}
