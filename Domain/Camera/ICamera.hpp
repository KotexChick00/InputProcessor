#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace Domain::Camera
{
	// Giao diện Camera (Interface)
	// - Cung cấp các phương thức để truy xuất thông tin camera và thiết lập phép chiếu
	class ICamera
	{
	public:
		virtual ~ICamera() = default;

		// Lấy Ma trận Quan sát (View Matrix) chuyển từ World Space sang Camera/View Space
		// Lưu ý: tham chiếu chỉ hợp lệ đến lần thay đổi camera kế tiếp; cần giữ lâu thì copy.
		virtual const glm::mat4& GetViewMatrix() const = 0;

		// Lấy Ma trận Chiếu (Projection Matrix) chuyển từ View Space sang Clip Space
		// Lưu ý: giống GetViewMatrix, không giữ tham chiếu qua lần thay đổi camera kế tiếp.
		virtual const glm::mat4& GetProjectionMatrix() const = 0;

	public:
		// Các vector dưới đây trả về THEO GIÁ TRỊ (12 byte, rẻ như tham chiếu) để giá trị
		// đã lấy không bị đổi ngầm khi camera cập nhật cache về sau.

		// C - Vị trí camera (World Space)
		virtual glm::vec3 GetPosition() const = 0;

		// Hướng camera đang nhìn tới (look direction), world space, đã normalize
		// Lưu ý: khác với quy ước "n" trong UVN model kinh điển (n = eye - target,
		// trỏ NGƯỢC hướng nhìn) — ở đây Forward = -n, để khớp trực giác thông thường
		// (Forward trỏ về hướng camera nhìn tới, giống cách glm::lookAt dùng center - eye)
		virtual glm::vec3 GetForward() const = 0;

		// V - Vector hướng lên (Up Vector)
		virtual glm::vec3 GetUp() const = 0;

		// U - Vector hướng sang phải (Right Vector)
		virtual glm::vec3 GetRight() const = 0;

	public:
		// Thiết lập phép chiếu phối cảnh (Perspective Projection)
		// - fovYRadians: Góc mở chiều dọc (Field of View) tính bằng Radian
		// - nearClip: Khoảng cách tới mặt phẳng cắt gần (Near Plane)
		// - farClip: Khoảng cách tới mặt phẳng cắt xa (Far Plane)
		virtual void SetPerspective(float fovYRadians, float nearClip, float farClip) = 0;

		// Cập nhật kích thước khung hiển thị (Viewport Dimensions) để tính toán lại Aspect Ratio (width / height)
		virtual void SetViewportSize(uint32_t width, uint32_t height) = 0;
	};
} // namespace Domain::Camera