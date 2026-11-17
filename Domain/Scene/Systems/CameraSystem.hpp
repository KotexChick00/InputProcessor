#pragma once
#include <ECS/ISystem.hpp>
#include <Input/InputState.hpp>
#include <Scene/Scene.hpp>

#include <cstdint>

namespace Domain::Scene {
	// Điều khiển camera đang Active bằng input (orbit/pan/dolly) và giữ viewport cho mọi camera.
	// Chỉ camera Active đầu tiên nhận input, để mỗi lúc chỉ có một gesture dùng mouse delta.
	class CameraSystem : public CoreEngine::ECS::ISystem {
	public:
		// input == nullptr: không tạo controller (dùng cho test không có cửa sổ).
		explicit CameraSystem(const CoreEngine::InputState* input)
			: ISystem(CoreEngine::ECS::ESystemPriority::High), mInput(input) {}

		void OnUpdate(float deltaTime) override;

		// Gọi khi cửa sổ đổi kích thước; áp ngay cho mọi camera và camera tạo sau này.
		void SetViewportSize(uint32_t width, uint32_t height);
		uint32_t GetViewportWidth() const { return mWidth; }
		uint32_t GetViewportHeight() const { return mHeight; }

		// Tạo entity "Camera" nhìn bao trọn bounds (bounds không hợp lệ -> camera mặc định).
		Actor CreateCamera(Scene& scene, const Model::AssimpBoundingBox* bounds) const;
		// Đặt lại camera để nhìn bao trọn bounds (phím R).
		void FitToBounds(CameraComponent& camera, const Model::AssimpBoundingBox& bounds) const;

		// Camera Active đầu tiên, nullptr nếu không có.
		static CameraComponent* GetActiveCamera();

	private:
		const CoreEngine::InputState* mInput = nullptr;
		uint32_t mWidth = 1;
		uint32_t mHeight = 1;
	};
}
