#pragma once
#include <ECS/ISystem.hpp>
#include <Scene/Components.hpp>

#include <vector>

namespace Domain::Scene {
	// Phải khớp MAX_LIGHTS trong shader của RenderSystem.
	constexpr size_t MAX_SHADER_LIGHTS = 8;

	// Một đèn ở dạng shader cần (toạ độ thế giới).
	struct ShaderLight {
		Model::LightType Type = Model::LightType::Directional;
		glm::vec3 Position{ 0.0f };
		glm::vec3 Direction{ 0.0f, 0.0f, -1.0f };
		glm::vec3 Color{ 1.0f };
		float CosInnerCone = 1.0f;
		float CosOuterCone = 0.0f;
	};

	// Gom các LightComponent đang bật thành danh sách cho shader (tối đa MAX_SHADER_LIGHTS).
	// Bản A: LightComponent đã giữ toạ độ thế giới nên dùng trực tiếp.
	class LightingSystem : public CoreEngine::ECS::ISystem {
	public:
		LightingSystem() : ISystem(CoreEngine::ECS::ESystemPriority::Medium) {}

		void OnUpdate(float deltaTime) override;

		const std::vector<ShaderLight>& GetLights() const { return mLights; }
		// Số đèn bị bỏ ở frame gần nhất vì vượt MAX_SHADER_LIGHTS.
		size_t GetDroppedCount() const { return mDropped; }

	private:
		std::vector<ShaderLight> mLights;
		size_t mDropped = 0;
		bool mWarnedAboutDropped = false;
	};
}
