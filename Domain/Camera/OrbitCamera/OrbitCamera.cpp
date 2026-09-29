#include <pch.h>
#include <Camera/OrbitCamera/OrbitCamera.hpp>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
	constexpr float kTwoPi = glm::two_pi<float>();
	constexpr float kPi = glm::pi<float>();
	constexpr float kHalfPi = glm::half_pi<float>();

	constexpr float kElevationEpsilon = 0.01f; // Giữ elevation cách ±90° một đoạn nhỏ để cross(forward, worldUp) không về 0
	constexpr float kMinZoomDistance = 0.1f;   // Khoảng cách tối thiểu từ camera đến target
	constexpr float kMaxZoomDistance = 1000.0f; // Khoảng cách tối đa từ camera đến target

	// Đưa góc về [-π, π] để azimuth không tăng vô hạn (mất độ chính xác float khi quay lâu)
	float WrapAngle(float angle)
	{
		return std::remainder(angle, kTwoPi);
	}

	float ClampElevation(float elevation)
	{
		return glm::clamp(elevation, -kHalfPi + kElevationEpsilon, kHalfPi - kElevationEpsilon);
	}
}

namespace Domain::Camera::OrbitCamera
{
	OrbitCamera::OrbitCamera(const glm::vec3& target, float distance, float azimuth, float elevation)
		: m_target(target)
		, m_distance(glm::clamp(distance, kMinZoomDistance, kMaxZoomDistance))
		, m_azimuth(WrapAngle(azimuth))
		, m_elevation(ClampElevation(elevation))
		, m_initialTarget(m_target)
		, m_initialDistance(m_distance)
		, m_initialAzimuth(m_azimuth)
		, m_initialElevation(m_elevation)
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
		m_azimuth = WrapAngle(m_azimuth + deltaAzimuth);
		m_elevation = ClampElevation(m_elevation + deltaElevation);
		m_viewDirty = true;
	}
	void OrbitCamera::Zoom(float delta)
	{
		m_distance = glm::clamp(m_distance - delta, kMinZoomDistance, kMaxZoomDistance);
		m_viewDirty = true;
	}
	void OrbitCamera::Reset()
	{
		m_target = m_initialTarget;
		m_distance = m_initialDistance;
		m_azimuth = m_initialAzimuth;
		m_elevation = m_initialElevation;
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
	glm::vec3 OrbitCamera::GetPosition() const
	{
		RecalculateCameraVectors();
		return m_position;
	}
	glm::vec3 OrbitCamera::GetForward() const
	{
		RecalculateCameraVectors();
		return m_forward;
	}
	glm::vec3 OrbitCamera::GetUp() const
	{
		RecalculateCameraVectors();
		return m_up;
	}
	glm::vec3 OrbitCamera::GetRight() const
	{
		RecalculateCameraVectors();
		return m_right;
	}
	void OrbitCamera::SetPerspective(float fovY, float nearClip, float farClip)
	{
		// glm::perspective cho ra NaN/inf nếu tham số sai
		if (fovY <= 0.0f || fovY >= kPi || nearClip <= 0.0f || farClip <= nearClip)
			return;
		m_fovY = fovY;
		m_near = nearClip;
		m_far = farClip;
		m_projectionDirty = true;
	}
	void OrbitCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		// Cửa sổ bị minimize -> 0x0. Giữ giá trị cũ, đợi lần resize hợp lệ tiếp theo.
		if (width == 0 || height == 0) return;
		m_aspect = static_cast<float>(width) / static_cast<float>(height);
		m_viewportHeight = static_cast<float>(height);
		m_projectionDirty = true;
	}
	void OrbitCamera::SetAzimuth(float azimuth)
	{
		m_azimuth = WrapAngle(azimuth);
		m_viewDirty = true;
	}
	void OrbitCamera::SetElevation(float elevation)
	{
		m_elevation = ClampElevation(elevation);
		m_viewDirty = true;
	}
	void OrbitCamera::RecalculateCameraVectors() const
	{
		if (!m_viewDirty) return;

		const float cosElevation = std::cos(m_elevation);
		const float x = m_distance * cosElevation * std::sin(m_azimuth);
		const float y = m_distance * std::sin(m_elevation);
		const float z = m_distance * cosElevation * std::cos(m_azimuth);

		m_position = m_target + glm::vec3(x, y, z);
		m_forward = glm::normalize(m_target - m_position); // N - Hướng nhìn (Forward Vector)
		m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f))); // U - Right Vector
		m_up = glm::normalize(glm::cross(m_right, m_forward)); // V - Up Vector

		m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
		m_viewDirty = false;
	}
	void OrbitCamera::RecalculateProjection() const
	{
		if (!m_projectionDirty) return;
		m_projectionMatrix = glm::perspective(m_fovY, m_aspect, m_near, m_far);
		m_projectionDirty = false;
	}
} // namespace Domain::Camera::OrbitCamera