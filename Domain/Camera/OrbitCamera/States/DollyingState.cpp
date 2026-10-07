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

		if (!controller.IsGesturePressed(OrbitCameraAction::Dolly))
		{
			controller.RequestStateChange(std::make_unique<IdleState>());
			return;
		}

		const auto& inputState = controller.GetInputState();
		bool mouseActive = inputState.MouseInput && controller.GetInputMap().IsContinuousPressed(OrbitCameraAction::Dolly, inputState);
		float mouseDeltaY = mouseActive ? inputState.MouseInput->GetDeltaY() : 0;

		if (mouseDeltaY != 0.0f)
		{
			// Nhân theo hàm mũ: hệ số luôn > 0, kéo nhanh cũng không thể làm distance âm.
			// Kéo xuống (dy > 0) -> xa ra, kéo lên -> lại gần.
			auto& camera = controller.GetOrbitCamera();
			camera.SetDistance(camera.GetDistance() * std::exp(mouseDeltaY * kDollySensitivity));
		}

		auto& camera = controller.GetOrbitCamera();
		float direction = controller.GetActionAxis(OrbitCameraAction::DollyIn, OrbitCameraAction::DollyOut);
		camera.SetDistance(camera.GetDistance() * std::exp(direction * deltaTime));

	}
} // namespace Domain::Camera::OrbitCamera::States
