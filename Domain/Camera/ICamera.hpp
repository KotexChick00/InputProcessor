#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace Domain::Camera
{
	class ICamera
	{
	public:
		virtual ~ICamera() = default;

		// Lấy Ma trận Quan sát (View Matrix) chuyển từ World Space sang Camera/View Space
		virtual const glm::mat4& GetViewMatrix() const = 0;

		// Lấy Ma trận Chiếu (Projection Matrix) chuyển từ View Space sang Clip Space
		virtual const glm::mat4& GetProjectionMatrix() const = 0;

	public:
		// C - Vị trí camera (World Space)
		virtual const glm::vec3& GetPosition() const = 0; 

		// Hướng camera đang nhìn tới (look direction), world space, đã normalize
		// Lưu ý: khác với quy ước "n" trong UVN model kinh điển (n = eye - target,
		// trỏ NGƯỢC hướng nhìn) — ở đây Forward = -n, để khớp trực giác thông thường
		// (Forward trỏ về hướng camera nhìn tới, giống cách glm::lookAt dùng center - eye)
		virtual const glm::vec3& GetForward() const = 0;

		// V - Vector hướng lên (Up Vector)
		virtual const glm::vec3& GetUp() const = 0;

		// U - Vector hướng sang phải (Right Vector)
		virtual const glm::vec3& GetRight() const = 0;   

	public:
		// Thiết lập phép chiếu phối cảnh (Perspective Projection)
		// - fovYRadians: Góc mở chiều dọc (Field of View) tính bằng Radian
		// - nearClip: Khoảng cách tới mặt phẳng cắt gần (Near Plane)
		// - farClip: Khoảng cách tới mặt phẳng cắt xa (Far Plane)
		virtual void SetPerspective(float fovYRadians, float nearClip, float farClip) = 0;

		// Cập nhật kích thước khung hiển thị (Viewport Dimensions) để tính toán lại Aspect Ratio (width / height)
		virtual void SetViewportSize(uint32_t width, uint32_t height) = 0;
	};
}