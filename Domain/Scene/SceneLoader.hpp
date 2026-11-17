#pragma once
#include <Scene/Scene.hpp>
#include <Model/Importers/IModelImporter.hpp>

#include <string>

namespace Domain::Scene {
	// Hướng chiếu của đèn mặc định, khớp với hướng sáng cố định trước đây trong shader
	// (shader dùng vector hướng VỀ phía đèn normalize(1, 2, 3), nên ánh sáng đi theo chiều ngược lại).
	inline const glm::vec3 DEFAULT_LIGHT_DIRECTION = -glm::normalize(glm::vec3(1.0f, 2.0f, 3.0f));

	// Đọc file model và dựng entity trong Scene.
	// Bản A (giai đoạn 1): cả model là 1 entity, đỉnh đã áp sẵn node transform nên Transform = đơn vị.
	// Mỗi đèn trong file là 1 entity con của entity model; file không có đèn -> tạo 1 đèn mặc định.
	class SceneLoader {
	public:
		explicit SceneLoader(Model::IModelImporter& importer) : mImporter(importer) {}

		// Thành công: xoá nội dung cũ của scene (giữ Render Settings) và trả về entity model.
		// Thất bại (thiếu file, file hỏng...): scene giữ nguyên, trả về Actor không hợp lệ.
		Actor LoadModel(Scene& scene, const std::string& path);

	private:
		Model::IModelImporter& mImporter;
	};
}
