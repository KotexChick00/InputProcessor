#include <Camera/OrbitCamera/States/OrbitingState.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <memory>
#include <utility>

namespace
{
	constexpr float kOrbitSensitivity = 0.005f; // Độ nhạy xoay camera (radian trên pixel)
}

namespace Domain::Camera::OrbitCamera::States
{
	void OrbitingState::Update(OrbitCameraController& controller, float deltaTime)
	{
		// Có gesture khác vừa được nhấn đè lên -> nhường quyền cho nó, không áp delta frame này nữa.
		if (auto next = controller.TryCreateOverridingState(OrbitCameraAction::Orbit))
		{
			controller.RequestStateChange(std::move(next));
			return;
		}

		if (!controller.IsGesturePressed(OrbitCameraAction::Orbit))
		{
			controller.RequestStateChange(std::make_unique<IdleState>());
			return;
		}

		const auto& inputState = controller.GetInputState();

		bool mouseActive = inputState.MouseInput && controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Orbit, inputState);
		float mouseDeltaX = mouseActive ? inputState.MouseInput->GetDeltaX() : 0;
		float mouseDeltaY = mouseActive ? inputState.MouseInput->GetDeltaY() : 0;

		if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
		{
			controller.GetOrbitCamera().Rotate(
				-mouseDeltaX * kOrbitSensitivity,
				-mouseDeltaY * kOrbitSensitivity);
		}

		// Keyboard speed is radians/second; mouse motion remains radians/pixel.
		controller.GetOrbitCamera().Rotate(
			controller.GetActionAxis(OrbitCameraAction::OrbitLeft, OrbitCameraAction::OrbitRight) * deltaTime,
			controller.GetActionAxis(OrbitCameraAction::OrbitDown, OrbitCameraAction::OrbitUp) * deltaTime);

	}
} // namespace Domain::Camera::OrbitCamera::States
