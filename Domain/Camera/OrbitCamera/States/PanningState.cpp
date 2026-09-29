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

		const auto& inputState = controller.GetInputState();
		float mouseDeltaX = inputState.MouseInput->GetDeltaX();
		float mouseDeltaY = inputState.MouseInput->GetDeltaY();
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

		// TODO: hỗ trợ người dùng pan bằng bàn phím

		if (!controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Pan, inputState))
		{
			// Khi người dùng thả nút Pan, trở về IdleState
			controller.RequestStateChange(std::make_unique<IdleState>());
		}
	}
} // namespace Domain::Camera::OrbitCamera::States