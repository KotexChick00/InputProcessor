#include <Camera/OrbitCamera/States/OrbitingState.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <memory>

namespace
{
	constexpr float kOrbitSensitivity = 0.005f; // Độ nhạy xoay camera (radian trên pixel)

}

namespace Domain::Camera::OrbitCamera::States
{
	void OrbitingState::OnEnter(OrbitCameraController& controller)
	{
		// Khi bắt đầu xoay, lock action Orbit để tránh tranh chấp với Pan/Dolly
		controller.LockAction(OrbitCameraAction::Orbit);
	}
	void OrbitingState::OnExit(OrbitCameraController& controller)
	{
		controller.UnlockAction();
	}
	void OrbitingState::Update(OrbitCameraController& controller, float deltaTime)
	{
		const auto& inputState = controller.GetInputState();

		float mouseDeltaX = inputState.MouseInput->GetDeltaX();
		float mouseDeltaY = inputState.MouseInput->GetDeltaY();

		if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
		{
			controller.GetOrbitCamera().Rotate(
				-mouseDeltaX * kOrbitSensitivity,
				-mouseDeltaY * kOrbitSensitivity);
		}

		// TODO: hỗ trợ người dùng rotate bằng bàn phím

		if (!controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Orbit, inputState))
		{
			// Khi người dùng thả nút Orbit, trở về IdleState
			controller.ChangeState(std::make_unique<IdleState>());
		}

	}
} // namespace Domain::Camera::OrbitCamera::States