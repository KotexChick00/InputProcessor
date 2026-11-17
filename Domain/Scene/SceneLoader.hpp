#pragma once
#include <Scene/Scene.hpp>
#include <Model/ImportedScene.hpp>

#include <string>

namespace Domain::Scene {
	// Hướng chiếu của đèn mặc định, khớp với hướng sáng cố định trước đây trong shader
	// (shader dùng vector hướng VỀ phía đèn normalize(1, 2, 3), nên ánh sáng đi theo chiều ngược lại).
	inline const glm::vec3 DEFAULT_LIGHT_DIRECTION = -glm::normalize(glm::vec3(1.0f, 2.0f, 3.0f));

	// Mặc định tối đa số node được tách thành entity. Mỗi loại component chứa tối đa 4096 cái
	// (64 chunk x 64), chừa chỗ cho camera, đèn mặc định và Render Settings.
	constexpr size_t DEFAULT_MAX_NODE_ENTITIES = 4000;

	// Đọc file model và dựng entity trong Scene theo cây node của file (bản B):
	//   - node gốc -> entity mang tên file, có ModelSource + Bounds (toạ độ thế giới)
	//   - mỗi node -> 1 entity giữ transform cục bộ; node có mesh thêm MeshRenderer (đỉnh cục bộ)
	//   - đèn gắn vào entity của node chứa nó (đèn thứ hai trở đi của cùng node -> entity con)
	//   - file không có đèn -> tạo "Default Light" (entity gốc riêng, không kế thừa transform của model)
	// File nhiều node hơn maxNodeEntities -> gộp thành 1 entity (như bản A) để không vượt giới hạn ECS.
	class SceneLoader {
	public:
		explicit SceneLoader(Model::ISceneImporter& importer, size_t maxNodeEntities = DEFAULT_MAX_NODE_ENTITIES)
			: mImporter(importer), mMaxNodeEntities(maxNodeEntities) {}

		// Thành công: xoá nội dung cũ của scene (giữ Render Settings) và trả về entity gốc của model.
		// Thất bại (thiếu file, file hỏng...): scene giữ nguyên, trả về Actor không hợp lệ.
		// Riêng khi hết bộ nhớ component giữa chừng: scene bị xoá trống (model cũ đã bị gỡ).
		Actor LoadModel(Scene& scene, const std::string& path);

	private:
		Model::ISceneImporter& mImporter;
		size_t mMaxNodeEntities;
	};
}
