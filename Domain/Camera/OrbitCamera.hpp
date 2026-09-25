#pragma once
#include <cstdint>
#include <Camera/ICamera.hpp>

namespace Domain::Camera
{
    // Camera dạng quỹ đạo (Orbit/Arcball Camera) - xoay quanh một điểm target cố định
    class OrbitCamera : public ICamera
    {
    public:
        // Khởi tạo camera với điểm nhìn trung tâm (target) và khoảng cách bán kính (distance)
        OrbitCamera(const glm::vec3& target, float distance);

        /* --- API riêng của OrbitCamera(không thuộc ICamera interface) -- - */

        // Thay đổi vị trí điểm mục tiêu mà camera đang hướng vào
        void SetTarget(const glm::vec3& target);

        // Cập nhật trực tiếp khoảng cách từ camera đến điểm target
        void SetDistance(float distance);

        // Xoay camera theo góc Azimuth (ngang) và Elevation (dọc) tính bằng Radian
        void Rotate(float deltaAzimuth, float deltaElevation);

        // Thu phóng khoảng cách đến target (thay đổi m_distance)
        void Zoom(float delta);

        /*--- Override các phương thức từ ICamera ---*/

        // Lấy Ma trận Quan sát (View Matrix) - tự động tính lại nếu dữ liệu bị "dirty"
        const glm::mat4& GetViewMatrix() const override;

        // Lấy Ma trận Chiếu (Projection Matrix) - tự động tính lại nếu dữ liệu bị "dirty"
        const glm::mat4& GetProjectionMatrix() const override;

        // Lấy vị trí hiện tại của Camera trong World Space (C)
        const glm::vec3& GetPosition() const override;

        // Lấy Vector hướng nhìn (Forward Vector / N)
        const glm::vec3& GetForward()   const override;

        // Lấy Vector hướng lên (Up Vector / V)
        const glm::vec3& GetUp()        const override;

        // Lấy Vector hướng sang phải (Right Vector / U)
        const glm::vec3& GetRight()     const override;

        // Thiết lập các thông số chiếu phối cảnh (FOV, Near plane, Far plane)
        void SetPerspective(float fovY, float nearClip, float farClip) override;

        // Cập nhật kích thước Viewport để tính lại Aspect Ratio (Tỷ lệ khung hình)
        void SetViewportSize(uint32_t width, uint32_t height) override;

    private:
        // Cập nhật vị trí (C) và hệ tọa độ UVN (m_forward, m_up, m_right, m_viewMatrix) từ các góc xoay và target
        void RecalculateCameraVectors() const;

        // Cập nhật lại m_projectionMatrix dựa trên fov, aspect ratio, near/far clip
        void RecalculateProjection() const;

        


    private:
        /*--- Thuộc tính trạng thái của Orbit Camera ---*/

        // Điểm trung tâm quan sát (Target Point)
        glm::vec3 m_target{0.0f, 0.0f, 0.0f};
        float m_distance{5.0f};       // Khoảng cách từ Camera đến Target
        float m_azimuth = 0.0f;   // Góc phương vị, góc xoay quanh trục Y (góc ngang / Yaw)
        float m_elevation = 0.0f;   // Góc nâng lên / xuống(Pitch), clamp trong(-90°, 90° ) để tránh N song song world-up (suy biến khi tính U = cross(N, worldUp))

        /*--- Thuộc tính phép chiếu(Projection) -- -*/
        
        // Góc mở chiều dọc (FOV Y)
        float m_fovY = glm::radians(45.0f); 
        float m_near = 0.1f, m_far = 1000.0f; // Mặt phẳng cắt gần/xa
        float m_aspect = 16.0f / 9.0f;      // Tỷ lệ khung hình (Width / Height)

        // --- Caching & Lazy Evaluation (mutable để cập nhật được trong các hàm const) ---
        mutable bool m_viewDirty = true; // Cờ đánh dấu View Matrix cần tính toán lại
        mutable bool m_projectionDirty = true; // Cờ đánh dấu Projection Matrix cần tính toán lại

		mutable glm::vec3 m_position{ 0.0f, 0.0f, 0.0f }; // Vị trí camera trong World Space (C)
		mutable glm::vec3 m_forward{ 0.0f, 0.0f, -1.0f }; // Hướng nhìn (Forward Vector / N), đã normalize
		mutable glm::vec3 m_up{ 0.0f, 1.0f, 0.0f }; // Vector hướng lên (Up Vector / V), đã normalize
		mutable glm::vec3 m_right{ 1.0f, 0.0f, 0.0f }; // Vector hướng sang phải (Right Vector / U), đã normalize

		mutable glm::mat4 m_viewMatrix{ 1.0f }; // Ma trận Quan sát (View Matrix) chuyển từ World Space sang Camera/View Space
		mutable glm::mat4 m_projectionMatrix{ 1.0f }; // Ma trận Chiếu (Projection Matrix) chuyển từ View Space sang Clip Space
    };
}