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
			controller.RequestStateChange(std::make_unique<IdleState>());
		}
	}
} // namespace Domain::Camera::OrbitCamera::States