#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Model {
	enum class LightType {
		Directional,
		Point,
		Spot
	};

	// Nguồn sáng đọc từ file model.
	// RenderModel::GetLights(): toạ độ thế giới (đã áp node transform).
	// ImportedNode::Lights: toạ độ cục bộ của node chứa đèn.
	struct ImportedLight {
		std::string Name;
		LightType Type = LightType::Directional;
		glm::vec3 Position{ 0.0f };
		// Hướng ánh sáng đi tới (không phải hướng về phía đèn). Point light không dùng.
		glm::vec3 Direction{ 0.0f, 0.0f, -1.0f };
		// Màu đã nhân sẵn cường độ (Assimp gộp glTF `intensity` vào màu).
		glm::vec3 Color{ 1.0f };
		// Nửa góc nón (radian) như glTF KHR_lights_punctual. Chỉ dùng cho spot light.
		float InnerConeAngle = 0.0f;
		float OuterConeAngle = 0.0f;
	};
}
