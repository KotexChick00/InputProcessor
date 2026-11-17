#pragma once
#include <Scene/Scene.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Lưu / load "trạng thái xem" (giai đoạn 5).
// Chỉ lưu cách đang XEM model: đường dẫn model, camera, tuỳ chọn hiển thị, node bị ẩn, đèn bị tắt.
// Không lưu hình học hay transform, không bao giờ ghi vào file model (đặc tả 1.5: không sửa / export model).
namespace Domain::Scene {
	constexpr int VIEW_STATE_VERSION = 1;

	struct ViewState {
		// Như ghi trong file: tương đối so với thư mục chứa file trạng thái, hoặc tuyệt đối.
		std::string ModelPath;

		bool HasCamera = false;
		glm::vec3 CameraTarget{ 0.0f };
		float CameraDistance = 5.0f;
		float CameraAzimuth = 0.0f;   // radian
		float CameraElevation = 0.0f; // radian

		bool Wireframe = false;
		bool Textures = true;
		bool Lighting = true;

		// Đường dẫn node tính từ entity gốc của model, vd "Rig/Spot" (xem GetNodePath).
		std::vector<std::string> HiddenNodes;
		std::vector<std::string> DisabledLights;
	};

	// --- JSON ---
	std::string ViewStateToJson(const ViewState& state);
	// nullopt + error khi JSON hỏng, sai kiểu, thiếu "model" hoặc version không hỗ trợ.
	std::optional<ViewState> ViewStateFromJson(std::string_view json, std::string& error);

	bool SaveViewState(const ViewState& state, const std::filesystem::path& file, std::string& error);
	std::optional<ViewState> LoadViewState(const std::filesystem::path& file, std::string& error);

	// --- Đường dẫn node ---
	// Tên các node từ dưới entity gốc xuống, nối bằng '/'. Gốc -> "". Không thuộc cây của root -> nullopt.
	// Anh em trùng tên được đánh số: "Wheel", "Wheel#1", "Wheel#2"...
	std::optional<std::string> GetNodePath(Actor entity, Actor root);
	Actor FindByNodePath(Actor root, std::string_view path);

	// --- Scene <-> ViewState ---
	// viewFile: nơi sẽ lưu, để ghi đường dẫn model tương đối với nó.
	ViewState CaptureViewState(const Scene& scene, Actor modelRoot, const CameraComponent* camera,
		const std::filesystem::path& viewFile);
	// Đường dẫn model thật từ ViewState (giải tương đối theo thư mục của viewFile).
	std::filesystem::path ResolveModelPath(const ViewState& state, const std::filesystem::path& viewFile);
	// Áp lên scene đã nạp đúng model. Node/đèn không tìm thấy thì bỏ qua (trả về số mục bị bỏ qua).
	size_t ApplyViewState(const ViewState& state, Scene& scene, Actor modelRoot, CameraComponent* camera);
}
