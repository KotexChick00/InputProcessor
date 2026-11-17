#pragma once
#include <Scene/Scene.hpp>
#include <Scene/SceneLoader.hpp>
#include <Scene/ViewState.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Domain::Scene {
	// Thông tin một node của model để GUI hiển thị cây cấu trúc (chỉ đọc).
	struct EntityInfo {
		EntityID Id = INVALID_ENTITY_ID;
		EntityID Parent = INVALID_ENTITY_ID; // INVALID_ENTITY_ID: node gốc của model
		std::string Name;
		std::string Path;                    // đường dẫn node, vd "Rig/Spot" (gốc = "")
		int Depth = 0;                       // độ sâu trong cây, gốc = 0

		size_t MeshCount = 0;
		size_t VertexCount = 0;
		size_t TriangleCount = 0;
		bool Visible = true;                 // chỉ có nghĩa khi MeshCount > 0

		bool HasLight = false;
		Model::LightType LightType = Model::LightType::Directional;
		bool LightEnabled = true;
	};

	// Thống kê toàn model (thanh trạng thái của GUI).
	struct ModelStats {
		std::string Path;
		size_t NodeCount = 0;
		size_t MeshCount = 0;
		size_t VertexCount = 0;
		size_t TriangleCount = 0;
		size_t LightCount = 0;  // đèn trong file (không tính đèn mặc định)
		bool HasBounds = false;
		glm::vec3 BoundsMin{ 0.0f };
		glm::vec3 BoundsMax{ 0.0f };
	};

	// API cho tầng ứng dụng / GUI: mở model, lưu/nạp trạng thái xem, cây node, bật tắt hiển thị.
	// Chỉ đọc và trình diễn: không có thao tác sửa hình học hay transform của model (đặc tả 1.5).
	//
	// Cần CameraSystem đã được thêm vào ECS (AddViewerSystems). GUI viết trong Core không được gọi
	// trực tiếp lớp này (Core không phụ thuộc Domain): nối GUI với ViewerController ở tầng ứng dụng (main.cpp).
	class ViewerController {
	public:
		ViewerController(CoreEngine::IResourceManager* resources, CameraSystem* cameras);
		~ViewerController();

		ViewerController(const ViewerController&) = delete;
		ViewerController& operator=(const ViewerController&) = delete;

		// --- Mở / lưu / nạp. Đường dẫn là chuỗi UTF-8. Thất bại: trả về false, error mô tả lỗi,
		// cảnh hiện tại giữ nguyên; không ném exception. ---
		bool OpenModel(const std::string& path, std::string& error);
		bool SaveView(const std::string& file, std::string& error) const;
		bool LoadView(const std::string& file, std::string& error);

		bool HasModel() const { return mRoot.IsValid(); }
		// Tăng mỗi lần mở model/nạp trạng thái thành công. EntityID cũ không còn hợp lệ khi số này đổi
		// (ECS tái dùng id), GUI phải lấy lại GetEntityTree().
		uint64_t GetModelGeneration() const { return mGeneration; }
		const std::string& GetModelPath() const { return mModelPath; }

		// --- Thông tin cấu trúc (chỉ đọc) ---
		// Cây node của model theo thứ tự duyệt sâu (cha đứng trước con).
		std::vector<EntityInfo> GetEntityTree() const;
		ModelStats GetModelStats() const;

		// --- Tuỳ chọn hiển thị ---
		// Ẩn/hiện node; recursive = true áp cho cả nhánh con. false nếu id không thuộc model.
		bool SetVisible(EntityID id, bool visible, bool recursive = true);
		bool SetLightEnabled(EntityID id, bool enabled);
		RenderSettingsComponent& GetRenderSettings() { return mScene->GetRenderSettings(); }
		const RenderSettingsComponent& GetRenderSettings() const { return mScene->GetRenderSettings(); }

		// Đưa camera về nhìn bao trọn model.
		void FitCamera();

		Scene& GetScene() { return *mScene; }
		Actor GetModelRoot() const { return mRoot; }

	private:
		bool OpenModelUnguarded(const std::string& path, std::string& error);
		bool IsInModel(Actor entity) const;
		void SetVisibleRecursive(Actor entity, bool visible, bool recursive);
		void CollectTree(Actor entity, int depth, std::vector<EntityInfo>& out) const;

	private:
		CameraSystem* mCameras = nullptr; // Thuộc SystemManager
		std::unique_ptr<Model::AssimpModelImporter> mImporter;
		std::unique_ptr<Scene> mScene;
		Actor mRoot;
		std::string mModelPath;
		uint64_t mGeneration = 0;
	};
}
