#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/States/OrbitingState.hpp>
#include <Camera/OrbitCamera/States/PanningState.hpp>
#include <Camera/OrbitCamera/States/DollyingState.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>

#include <memory>

namespace Domain::Camera::OrbitCamera::States
{
	void IdleState::Update(OrbitCameraController& controller, float deltaTime)
	{
		// Nếu đang bị lock bởi 1 action khác, thì IdleState không làm gì cả.
		if (controller.IsLocked())
			return;
		const auto& inputState = controller.GetInputState();
		const auto& inputMap = controller.GetInputMap();
		// Kiểm tra các action theo thứ tự ưu tiên: Orbit > Pan > Dolly
		
		if (inputMap.IsJustPressed(OrbitCameraAction::Orbit, inputState))
		{
			controller.ChangeState(std::make_unique<OrbitingState>());
			return;
		}
		else if (inputMap.IsJustPressed(OrbitCameraAction::Pan, inputState))
		{
			controller.ChangeState(std::make_unique<PanningState>());
			return;
		}
		else if (inputMap.IsJustPressed(OrbitCameraAction::Dolly, inputState))
		{
			controller.ChangeState(std::make_unique<DollyingState>());
			return;
		}

		// Zoom/Reset being processed in OrbitCameraController::Update() directly, so no need to handle them here.
	}
} // namespace Domain::Camera::OrbitCamera::States