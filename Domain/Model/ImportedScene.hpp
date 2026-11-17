#pragma once
#include <Model/RenderModel.hpp>
#include <Model/ImportedLight.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Model {
	// Một node của file model, giữ nguyên transform cục bộ (không áp vào đỉnh).
	struct ImportedNode {
		std::string Name;
		int Parent = -1; // chỉ số node cha trong ImportedScene::Nodes, -1 = node gốc

		// Transform cục bộ so với node cha. Nếu IsTrs thì Local == T * R * S.
		glm::mat4 Local{ 1.0f };
		bool IsTrs = true; // false: ma trận có shear/không tách được TRS, phải dùng Local trực tiếp
		glm::vec3 Position{ 0.0f };
		glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f };

		// Các mesh của node, đỉnh theo toạ độ cục bộ của node. nullptr nếu node không có mesh.
		std::unique_ptr<RenderModel> Model;
		// Đèn gắn với node, vị trí/hướng theo toạ độ cục bộ của node.
		std::vector<ImportedLight> Lights;
	};

	// Kết quả đọc file theo cây node. Node cha luôn đứng trước node con trong Nodes.
	struct ImportedScene {
		std::vector<ImportedNode> Nodes;
		// true: file quá nhiều node nên đã gộp thành 1 node với đỉnh/đèn theo toạ độ thế giới (như bản A).
		bool Flattened = false;
	};

	class ISceneImporter {
	public:
		virtual ~ISceneImporter() = default;
		// nullptr nếu không đọc được file.
		// Số node vượt maxNodes thì gộp thành 1 node (Flattened) để không vượt giới hạn component của ECS.
		virtual std::unique_ptr<ImportedScene> ImportScene(const char* file, size_t maxNodes) = 0;
	};
}
