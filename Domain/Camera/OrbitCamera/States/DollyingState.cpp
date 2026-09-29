#include <Camera/OrbitCamera/States/DollyingState.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <cmath>
#include <memory>
#include <utility>
#include <glm/glm.hpp>

namespace
{
	constexpr float kDollySensitivity = 0.02f; // Hệ số mũ trên mỗi pixel (distance nhân e^(0.02*px))
}

namespace Domain::Camera::OrbitCamera::States
{
	void DollyingState::Update(OrbitCameraController& controller, float deltaTime)
	{
		// Có gesture khác vừa được nhấn đè lên -> nhường quyền cho nó, không áp delta frame này nữa.
		if (auto next = controller.TryCreateOverridingState(OrbitCameraAction::Dolly))
		{
			controller.RequestStateChange(std::move(next));
			return;
		}

		const auto& inputState = controller.GetInputState();
		float mouseDeltaY = inputState.MouseInput->GetDeltaY();

		if (mouseDeltaY != 0.0f)
		{
			// Nhân theo hàm mũ: hệ số luôn > 0, kéo nhanh cũng không thể làm distance âm.
			// Kéo xuống (dy > 0) -> xa ra, kéo lên -> lại gần.
			auto& camera = controller.GetOrbitCamera();
			camera.SetDistance(camera.GetDistance() * std::exp(mouseDeltaY * kDollySensitivity));
		}

		// TODO: hỗ trợ người dùng dolly bằng bàn phím

		if (!controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Dolly, inputState))
		{
			// Khi người dùng thả nút Dolly, trở về IdleState
			controller.RequestStateChange(std::make_unique<IdleState>());
		}
	}
} // namespace Domain::Camera::OrbitCamera::States