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
		// Dùng IsContinuousPressed (không phải IsJustPressed) để không nuốt input:
		// giữ Pan, nhấn Orbit (Orbit thắng), rồi thả Orbit thì Pan vẫn được kích hoạt
		// ngay ở frame sau. Thứ tự ưu tiên này chỉ áp dụng khi nhiều nút cùng được giữ
		// lúc rảnh; khi đang có gesture, gesture nhấn sau sẽ thắng (xem TryCreateOverridingState).
		if (controller.IsGesturePressed(OrbitCameraAction::Orbit))
		{
			controller.RequestStateChange(std::make_unique<OrbitingState>());
			return;
		}
		else if (controller.IsGesturePressed(OrbitCameraAction::Pan))
		{
			controller.RequestStateChange(std::make_unique<PanningState>());
			return;
		}
		else if (controller.IsGesturePressed(OrbitCameraAction::Dolly))
		{
			controller.RequestStateChange(std::make_unique<DollyingState>());
			return;
		}
	}
} // namespace Domain::Camera::OrbitCamera::States
