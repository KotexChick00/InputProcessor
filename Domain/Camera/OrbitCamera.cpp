#include <Camera/OrbitCamera.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
	constexpr float kEpsilon = glm::epsilon<float>(); // Độ lệch nhỏ để so sánh float

	constexpr float kTwoPi = glm::two_pi<float>(); // Góc 360 độ (2π radian)

	constexpr float kPi = glm::pi<float>(); // Góc 180 độ (π radian)
	constexpr float kNegativePi = -glm::pi<float>(); // Góc -180 độ (-π radian)

	constexpr float kHalfPi = glm::half_pi<float>(); // Góc 90 độ (π/2 radian)
	constexpr float kNegativeHalfPi = -glm::half_pi<float>(); // Góc -90 độ (-π/2 radian)

	constexpr float kElevationEpsilon = 0.01f; // Độ lệch nhỏ để tránh gimbal lock khi elevation gần ±90°
	constexpr float kMinZoomDistance = 0.1f; // Khoảng cách tối thiểu từ camera đến target
	constexpr float kMaxZoomDistance = 1000.0f; // Khoảng cách tối đa từ camera đến target
}

namespace Domain::Camera
{

	OrbitCamera::OrbitCamera(const glm::vec3& target, float distance)
		: m_target(target)
		, m_distance(glm::clamp(distance, kMinZoomDistance, kMaxZoomDistance))
		, m_position(0.0f, 0.0f, 0.0f)
		, m_forward(0.0f, 0.0f, -1.0f)
		, m_up(0.0f, 1.0f, 0.0f)
		, m_right(1.0f, 0.0f, 0.0f)
		, m_viewMatrix(1.0f)
		, m_projectionMatrix(1.0f)
	{}
	void OrbitCamera::SetTarget(const glm::vec3& target)
	{
		m_target = target;
		m_viewDirty = true;
	}
	void OrbitCamera::SetDistance(float distance)
	{
		m_distance = glm::clamp(distance, kMinZoomDistance, kMaxZoomDistance);
		m_viewDirty = true;
	}
	void OrbitCamera::Rotate(float deltaAzimuth, float deltaElevation)
	{
		m_azimuth += deltaAzimuth;
		m_elevation = glm::clamp(m_elevation + deltaElevation, kNegativeHalfPi + kElevationEpsilon, kHalfPi - kElevationEpsilon);

		m_viewDirty = true;
	}
	void OrbitCamera::Zoom(float delta)
	{
		m_distance = glm::clamp(m_distance - delta, kMinZoomDistance, kMaxZoomDistance);
		m_viewDirty = true;
	}
	const glm::mat4& OrbitCamera::GetViewMatrix() const
	{
		RecalculateCameraVectors();
		return m_viewMatrix;
	}
	const glm::mat4& OrbitCamera::GetProjectionMatrix() const
	{
		RecalculateProjection();
		return m_projectionMatrix;
	}
	const glm::vec3& OrbitCamera::GetPosition() const
	{
		RecalculateCameraVectors();
		return m_position;
	}
	const glm::vec3& OrbitCamera::GetForward() const
	{
		RecalculateCameraVectors();
		return m_forward;
	}
	const glm::vec3& OrbitCamera::GetUp() const
	{
		RecalculateCameraVectors();
		return m_up;
	}
	const glm::vec3& OrbitCamera::GetRight() const
	{
		RecalculateCameraVectors();
		return m_right;
	}
	void OrbitCamera::SetPerspective(float fovY, float nearClip, float farClip)
	{
		m_fovY = fovY;
		m_near = nearClip;
		m_far = farClip;
		m_projectionDirty = true;
	}
	void OrbitCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_aspect = static_cast<float>(width) / static_cast<float>(height);
		m_projectionDirty = true;
	}
	void OrbitCamera::RecalculateCameraVectors() const
	{
		if (!m_viewDirty) return;

		float x = m_distance * cos(m_elevation) * sin(m_azimuth);
		float y = m_distance * sin(m_elevation);
		float z = m_distance * cos(m_elevation) * cos(m_azimuth);

		m_position = m_target + glm::vec3(x, y, z);
		m_forward = glm::normalize(m_target - m_position); // N - Hướng nhìn (Forward Vector)
		m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f))); // U - Vector hướng sang phải (Right Vector)
		m_up = glm::cross(m_right, m_forward); // V - Vector hướng lên (Up Vector)

		m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
		m_viewDirty = false;
	}
	void OrbitCamera::RecalculateProjection() const
	{
		if (!m_projectionDirty) return;
		m_projectionMatrix = glm::perspective(m_fovY, m_aspect, m_near, m_far);
		m_projectionDirty = false;
	}
}