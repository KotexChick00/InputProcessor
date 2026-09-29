#pragma once
#include <memory>
#include <Camera/ICameraController.hpp>
#include <Camera/OrbitCamera/OrbitCamera.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/IOrbitControllerState.hpp>
#include <Camera/InputMap.hpp>
#include <Input/InputState.hpp>

namespace Domain::Camera::OrbitCamera
{
	// Controller điều khiển OrbitCamera bằng state machine (Idle/Orbiting/Panning/Dollying)
	// và InputMap<OrbitCameraAction> để người dùng có thể rebind nút tùy ý.
	// State machine đảm bảo mỗi lúc chỉ có 1 gesture dùng mouse-delta.
	class OrbitCameraController : public ICameraController
	{
	public:
		OrbitCameraController(OrbitCamera& camera, const CoreEngine::Input::InputState& inputState);

		void Update(float dt) override;
		ICamera& GetCamera() override { return m_camera; }

		// --- API dành cho các State gọi ngược lại ---

		// Chỉ đăng ký yêu cầu chuyển state; việc chuyển thật sự diễn ra
		// ở cuối Update() để state hiện tại không bị hủy khi đang chạy.
		void ChangeState(std::unique_ptr<IOrbitControllerState> next);

		// Nếu có gesture KHÁC `current` vừa được nhấn trong frame này thì trả về state tương ứng,
		// ngược lại trả nullptr. Dùng để gesture nhấn sau giành quyền từ gesture đang chạy.
		std::unique_ptr<IOrbitControllerState> TryCreateOverridingState(OrbitCameraAction current) const;

		OrbitCamera& GetOrbitCamera() { return m_camera; }
		const CoreEngine::Input::InputState& GetInputState() const { return m_inputState; }
		InputMap<OrbitCameraAction>& GetInputMap() { return m_inputMap; }
		const InputMap<OrbitCameraAction>& GetInputMap() const { return m_inputMap; }

	private:
		OrbitCamera& m_camera;
		const CoreEngine::Input::InputState& m_inputState;
		InputMap<OrbitCameraAction> m_inputMap;
		std::unique_ptr<IOrbitControllerState> m_state;
		std::unique_ptr<IOrbitControllerState> m_pendingState;
	};
} // namespace Domain::Camera::OrbitCamera