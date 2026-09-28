#include <Camera/OrbitCamera/OrbitCameraController.hpp>
#include <Camera/OrbitCamera/States/IdleState.hpp>

namespace
{
	constexpr float kZoomSpeed = 5.0f; // Đơn vị khoảng cách trên mỗi giây khi giữ ZoomIn/ZoomOut
}

namespace Domain::Camera::OrbitCamera
{
	OrbitCameraController::OrbitCameraController(OrbitCamera& camera, const CoreEngine::Input::InputState& inputState)
		: m_camera(camera)
		, m_inputState(inputState)
		, m_inputMap(CreateDefaultOrbitCameraInputMap())
		, m_state(std::make_unique<States::IdleState>())
	{}

	void OrbitCameraController::ChangeState(std::unique_ptr<IOrbitControllerState> next)
	{
		if (m_state) m_state->OnExit(*this);
		m_state = std::move(next);
		m_state->OnEnter(*this);
	}

	void OrbitCameraController::Update(float dt)
	{
		// ZoomIn/ZoomOut/Reset là action rời rạc, không tranh chấp mouse-delta
		// với Orbit/Pan/Dolly, nên xử lý trực tiếp mỗi frame - không cần Action Lock.
		if (m_inputMap.IsContinuousPressed(OrbitCameraAction::ZoomIn, m_inputState))
		{
			m_camera.Zoom(kZoomSpeed * dt);
		}
		if (m_inputMap.IsContinuousPressed(OrbitCameraAction::ZoomOut, m_inputState))
		{
			m_camera.Zoom(-kZoomSpeed * dt);
		}
		if (m_inputMap.IsJustPressed(OrbitCameraAction::Reset, m_inputState))
		{
			m_camera.Reset();
		}

		m_state->Update(*this, dt);
	}
} // namespace Domain::Camera::OrbitCamera