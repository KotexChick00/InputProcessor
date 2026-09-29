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
		const auto& inputState = controller.GetInputState();
		const auto& inputMap = controller.GetInputMap();

		// Dùng IsContinuousPressed (không phải IsJustPressed) để không nuốt input:
		// giữ Pan, nhấn Orbit (Orbit thắng), rồi thả Orbit thì Pan vẫn được kích hoạt
		// ngay ở frame sau. Thứ tự ưu tiên này chỉ áp dụng khi nhiều nút cùng được giữ
		// lúc rảnh; khi đang có gesture, gesture nhấn sau sẽ thắng (xem TryCreateOverridingState).
		if (inputMap.IsContinuousPressed(OrbitCameraAction::Orbit, inputState))
		{
			controller.RequestStateChange(std::make_unique<OrbitingState>());
			return;
		}
		else if (inputMap.IsContinuousPressed(OrbitCameraAction::Pan, inputState))
		{
			controller.RequestStateChange(std::make_unique<PanningState>());
			return;
		}
		else if (inputMap.IsContinuousPressed(OrbitCameraAction::Dolly, inputState))
		{
			controller.RequestStateChange(std::make_unique<DollyingState>());
			return;
		}
	}
} // namespace Domain::Camera::OrbitCamera::States