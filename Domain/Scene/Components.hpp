#pragma once
#include <ECS/IComponent.hpp>
#include <Model/RenderModel.hpp>
#include <Model/ImportedLight.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpBoundingBox.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <string>
#include <vector>

// Dữ liệu gắn vào entity của scene. Component chỉ chứa dữ liệu, logic nằm ở các System.
// Đặt trong Domain vì tham chiếu RenderModel/BoundingBox; Core/ECS không biết tới chúng.
namespace Domain::Scene {
	using CoreEngine::ECS::EntityID;
	using CoreEngine::ECS::INVALID_ENTITY_ID;

	struct NameComponent : CoreEngine::ECS::IComponent {
		std::string Name;

		explicit NameComponent(std::string name = {}) : Name(std::move(name)) {}
	};

	// Transform cục bộ (so với entity cha). World được TransformSystem tính lại khi Dirty.
	struct TransformComponent : CoreEngine::ECS::IComponent {
		glm::vec3 Position{ 0.0f };
		glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f };

		glm::mat4 World{ 1.0f };
		bool Dirty = true;

		glm::mat4 GetLocalMatrix() const {
			return glm::translate(glm::mat4(1.0f), Position)
				* glm::mat4_cast(Rotation)
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct HierarchyComponent : CoreEngine::ECS::IComponent {
		EntityID Parent = INVALID_ENTITY_ID;
		std::vector<EntityID> Children;
	};

	// Model cần vẽ. Sở hữu RenderModel: huỷ component sẽ giải phóng buffer GPU,
	// nên phải huỷ khi OpenGL context còn sống.
	struct MeshRendererComponent : CoreEngine::ECS::IComponent {
		std::unique_ptr<Model::RenderModel> Model;
		bool Visible = true;

		explicit MeshRendererComponent(std::unique_ptr<Model::RenderModel> model = nullptr)
			: Model(std::move(model)) {}
	};

	// Bounding box theo toạ độ thế giới, dùng để fit camera.
	struct BoundsComponent : CoreEngine::ECS::IComponent {
		Model::AssimpBoundingBox Box;
	};

	// Đường dẫn file model mà entity được tạo ra từ đó.
	struct ModelSourceComponent : CoreEngine::ECS::IComponent {
		std::string Path;

		explicit ModelSourceComponent(std::string path = {}) : Path(std::move(path)) {}
	};

	// Nguồn sáng (toạ độ thế giới). Direction là hướng ánh sáng đi tới.
	struct LightComponent : CoreEngine::ECS::IComponent {
		Model::ImportedLight Light;
		// true: lấy từ file; false: đèn mặc định do viewer tạo khi file không có đèn.
		bool FromFile = true;
		bool Enabled = true;
	};

	// Tuỳ chọn hiển thị chung cho cả scene ("bật tắt shader" trong đặc tả).
	struct RenderSettingsComponent : CoreEngine::ECS::IComponent {
		bool Wireframe = false;
		bool Textures = true;
		bool Lighting = true;
	};
}
