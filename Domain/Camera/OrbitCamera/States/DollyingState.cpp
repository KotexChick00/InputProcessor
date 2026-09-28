#include <Camera/OrbitCamera/States/DollyingState.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <memory>
#include <glm/glm.hpp>

namespace {
	constexpr float kDollySensitivity = 0.02f; // Độ nhạy dolly camera (đơn vị world trên pixel)
}

namespace Domain::Camera::OrbitCamera::States {
	void DollyingState::OnEnter(OrbitCameraController& controller) {
		// Khi bắt đầu dolly, lock action Dolly để tránh tranh chấp với Orbit/Pan
		controller.LockAction(OrbitCameraAction::Dolly);
	}
	void DollyingState::OnExit(OrbitCameraController& controller) {
		controller.UnlockAction();
	}
	void DollyingState::Update(OrbitCameraController& controller, float deltaTime) {
		const auto& inputState = controller.GetInputState();
		float mouseDeltaY = inputState.MouseInput->GetDeltaY();

		float zommScale = controller.GetOrbitCamera().GetDistance() * kDollySensitivity;

		if (mouseDeltaY != 0.0f) {
			controller.GetOrbitCamera().Zoom(- mouseDeltaY * zommScale);
		}

		// TODO: hỗ trợ người dùng dolly bằng bàn phím

		if (!controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Dolly, inputState)) {
			// Khi người dùng thả nút Dolly, trở về IdleState
			controller.ChangeState(std::make_unique<IdleState>());
		}
	}
} // namespace Domain::Camera::OrbitCamera::States