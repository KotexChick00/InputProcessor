#include <Camera/OrbitCamera/States/PanningState.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <memory>
#include <glm/glm.hpp>

namespace
{
	constexpr float kPanSensitivity = 0.01f; // Độ nhạy pan camera (đơn vị world trên pixel)
}

namespace Domain::Camera::OrbitCamera::States
{
	void PanningState::OnEnter(OrbitCameraController& controller)
	{
		// Khi bắt đầu pan, lock action Pan để tránh tranh chấp với Orbit/Dolly
		controller.LockAction(OrbitCameraAction::Pan);
	}
	void PanningState::OnExit(OrbitCameraController& controller)
	{
		controller.UnlockAction();
	}
	void PanningState::Update(OrbitCameraController& controller, float deltaTime)
	{
		const auto& inputState = controller.GetInputState();
		float mouseDeltaX = inputState.MouseInput->GetDeltaX();
		float mouseDeltaY = inputState.MouseInput->GetDeltaY();
		if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
		{
			auto& camera = controller.GetOrbitCamera();
			float distanceScale = camera.GetDistance() * kPanSensitivity;
			glm::vec3 offset = (-mouseDeltaX * camera.GetRight() + mouseDeltaY * camera.GetUp()) * distanceScale;
			camera.SetTarget(camera.GetTarget() + offset);
		}

		// TODO: hỗ trợ người dùng pan bằng bàn phím

		if (!controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Pan, inputState))
		{
			// Khi người dùng thả nút Pan, trở về IdleState
			controller.ChangeState(std::make_unique<IdleState>());
		}
	}
} // namespace Domain::Camera::OrbitCamera::States