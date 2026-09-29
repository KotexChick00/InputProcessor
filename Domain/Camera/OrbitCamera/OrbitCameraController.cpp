#include <cmath>
#include <utility>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>
#include <Camera/OrbitCamera/States/OrbitingState.hpp>
#include <Camera/OrbitCamera/States/PanningState.hpp>
#include <Camera/OrbitCamera/States/DollyingState.hpp>

namespace
{
	constexpr float kZoomFactorPerStep = 0.9f;     // Mỗi nấc scroll: khoảng cách nhân 0.9 (gần 10%)
	constexpr float kKeyZoomStepsPerSecond = 5.0f; // Nếu ZoomIn/Out bind vào phím: số "nấc" mỗi giây khi giữ
}

namespace Domain::Camera::OrbitCamera
{
	OrbitCameraController::OrbitCameraController(OrbitCamera& camera, const CoreEngine::Input::InputState& inputState)
		: m_camera(camera)
		, m_inputState(inputState)
		, m_inputMap(CreateDefaultOrbitCameraInputMap())
		, m_state(std::make_unique<States::IdleState>())
	{
		m_state->OnEnter(*this);
	}

	void OrbitCameraController::ChangeState(std::unique_ptr<IOrbitControllerState> next)
	{
		if (next) m_pendingState = std::move(next);
	}

	std::unique_ptr<IOrbitControllerState> OrbitCameraController::TryCreateOverridingState(OrbitCameraAction current) const
	{
		if (current != OrbitCameraAction::Orbit && m_inputMap.IsJustPressed(OrbitCameraAction::Orbit, m_inputState))
			return std::make_unique<States::OrbitingState>();
		if (current != OrbitCameraAction::Pan && m_inputMap.IsJustPressed(OrbitCameraAction::Pan, m_inputState))
			return std::make_unique<States::PanningState>();
		if (current != OrbitCameraAction::Dolly && m_inputMap.IsJustPressed(OrbitCameraAction::Dolly, m_inputState))
			return std::make_unique<States::DollyingState>();
		return nullptr;
	}

	void OrbitCameraController::Update(float dt)
	{
		// Zoom/Reset không dùng mouse-delta nên không tranh chấp với Orbit/Pan/Dolly
		// (state machine đảm bảo mỗi lúc chỉ có 1 gesture dùng delta), xử lý trực tiếp mỗi frame.
		float zoomSteps = 0.0f; // dương = lại gần
		auto accumulate = [&](OrbitCameraAction action, float sign)
		{
			if (!m_inputMap.IsContinuousPressed(action, m_inputState)) return;

			float amount = m_inputMap.IsScrollBinding(action)
				? std::abs(m_inputState.MouseInput->GetScrollY())
				: kKeyZoomStepsPerSecond * dt;
			zoomSteps += sign * amount;
		};
		accumulate(OrbitCameraAction::ZoomIn, +1.0f);
		accumulate(OrbitCameraAction::ZoomOut, -1.0f);
		if (zoomSteps != 0.0f)
		{
			m_camera.SetDistance(m_camera.GetDistance() * std::pow(kZoomFactorPerStep, zoomSteps));
		}
		if (m_inputMap.IsJustPressed(OrbitCameraAction::Reset, m_inputState))
		{
			m_camera.Reset();
		}

		m_state->Update(*this, dt);

		if (m_pendingState)
		{
			m_state->OnExit(*this);
			m_state = std::move(m_pendingState); // state cũ bị hủy ở đây, an toàn vì đã ra khỏi Update()
			m_state->OnEnter(*this);
		}
	}
} // namespace Domain::Camera::OrbitCamera