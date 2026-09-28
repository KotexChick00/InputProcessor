#pragma once
#include <memory>
#include <optional>
#include <Camera/ICameraController.hpp>
#include <Camera/OrbitCamera/OrbitCamera.hpp>
#include <Camera/OrbitCamera/OrbitCameraAction.hpp>
#include <Camera/OrbitCamera/IOrbitControllerState.hpp>
#include <Camera/InputMap.hpp>
#include <Input/InputState.hpp>

namespace Domain::Camera::OrbitCamera
{
	// Controller điều khiển OrbitCamera bằng state machine (Idle/Orbiting/Panning)
	// và InputMap<OrbitCameraAction> để người dùng có thể rebind nút tùy ý.
	class OrbitCameraController : public ICameraController
	{
	public:
		OrbitCameraController(OrbitCamera& camera, const CoreEngine::Input::InputState& inputState);

		void Update(float dt) override;
		ICamera& GetCamera() override { return m_camera; }

		// --- API dành cho các State gọi ngược lại ---

		void ChangeState(std::unique_ptr<IOrbitControllerState> next);

		OrbitCamera& GetOrbitCamera() { return m_camera; }
		const CoreEngine::Input::InputState& GetInputState() const { return m_inputState; }
		InputMap<OrbitCameraAction>& GetInputMap() { return m_inputMap; }

		// --- Action Lock: đảm bảo chỉ 1 action "sở hữu" gesture tại 1 thời điểm ---
		// Gọi trong OnEnter() của state khi bắt đầu 1 gesture (vd OrbitingState -> LockAction(Orbit)).
		// Trong khi bị khóa, mọi action khác (bất kể bind bằng mouse hay keyboard)
		// đều bị IdleState phớt lờ hoàn toàn - vì chính lock này quyết định
		// có được phép bắt đầu 1 gesture MỚI hay không, không phụ thuộc nguồn input.
		void LockAction(OrbitCameraAction action) { m_activeAction = action; }
		void UnlockAction() { m_activeAction.reset(); }
		bool IsLocked() const { return m_activeAction.has_value(); }
		bool IsLockedTo(OrbitCameraAction action) const { return m_activeAction == action; }

	private:
		OrbitCamera& m_camera;
		const CoreEngine::Input::InputState& m_inputState;
		InputMap<OrbitCameraAction> m_inputMap;
		std::unique_ptr<IOrbitControllerState> m_state;
		std::optional<OrbitCameraAction> m_activeAction; // action nào đang giữ quyền input, nếu có
	};
} // namespace Domain::Camera