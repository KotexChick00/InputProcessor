#include <Camera/OrbitCamera/States/PanningState.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <cmath>
#include <memory>
#include <utility>
#include <glm/glm.hpp>

namespace
{
	constexpr float kPanSpeed = 1.0f; // Hệ số nhân trên pan 1:1 (1.0 = điểm dưới con trỏ đi đúng theo chuột)
}

namespace Domain::Camera::OrbitCamera::States
{
	void PanningState::Update(OrbitCameraController& controller, float deltaTime)
	{
		// Có gesture khác vừa được nhấn đè lên -> nhường quyền cho nó, không áp delta frame này nữa.
		if (auto next = controller.TryCreateOverridingState(OrbitCameraAction::Pan))
		{
			controller.RequestStateChange(std::move(next));
			return;
		}

		if (!controller.IsGesturePressed(OrbitCameraAction::Pan))
		{
			controller.RequestStateChange(std::make_unique<IdleState>());
			return;
		}

		const auto& inputState = controller.GetInputState();
		bool mouseActive = inputState.MouseInput && controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Pan, inputState);
		float mouseDeltaX = mouseActive ? inputState.MouseInput->GetDeltaX() : 0;
		float mouseDeltaY = mouseActive ? inputState.MouseInput->GetDeltaY() : 0;
		if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
		{
			auto& camera = controller.GetOrbitCamera();

			float viewportHeight = camera.GetViewportHeight();

			if (viewportHeight <= 0.0f)
			{
				return;
			}

			// Chiều cao mặt phẳng nhìn thấy tại khoảng cách d là 2*d*tan(fov/2);
			// chia cho số pixel chiều cao viewport -> đơn vị world trên mỗi pixel.
			float worldPerPixel = 2.0f * camera.GetDistance() * std::tan(camera.GetFovY() * 0.5f)
				/ viewportHeight;
			glm::vec3 offset = (-mouseDeltaX * camera.GetRight() + mouseDeltaY * camera.GetUp())
				* (worldPerPixel * kPanSpeed);
			camera.SetTarget(camera.GetTarget() + offset);
		}

		// Move in the camera plane at a speed proportional to the viewing distance.
		auto& camera = controller.GetOrbitCamera();
		glm::vec3 keyboardOffset =
			camera.GetRight() * controller.GetActionAxis(OrbitCameraAction::PanLeft, OrbitCameraAction::PanRight)
			+ camera.GetUp() * controller.GetActionAxis(OrbitCameraAction::PanDown, OrbitCameraAction::PanUp);
		camera.SetTarget(camera.GetTarget() + keyboardOffset * camera.GetDistance() * deltaTime);

	}
} // namespace Domain::Camera::OrbitCamera::States
