#pragma once
#include <cstdint>
#include <Camera/ICamera.hpp>

#include <glm/glm.hpp>

namespace Domain::Camera::OrbitCamera
{
    // Camera dạng turntable (yaw quanh trục Y + pitch) - xoay quanh một điểm target.
    // Không phải arcball: không thể lật qua đỉnh, elevation bị clamp trong (-90°, 90°).
    class OrbitCamera : public ICamera
    {
    public:
        // Khởi tạo camera với điểm nhìn trung tâm (target), khoảng cách bán kính (distance)
        // và góc ban đầu (radian). Reset() sẽ đưa camera về đúng các giá trị này.
        OrbitCamera(const glm::vec3& target, float distance, float azimuth = 0.0f, float elevation = 0.0f);

        /* --- API riêng của OrbitCamera (không thuộc ICamera interface) --- */

        // Thay đổi vị trí điểm mục tiêu mà camera đang hướng vào
        void SetTarget(const glm::vec3& target);

        // Cập nhật trực tiếp khoảng cách từ camera đến điểm target
        void SetDistance(float distance);

        // Xoay camera theo góc Azimuth (ngang) và Elevation (dọc) tính bằng Radian
        void Rotate(float deltaAzimuth, float deltaElevation);

        // Thu phóng khoảng cách đến target (tuyến tính: m_distance -= delta)
        void Zoom(float delta);

        // Đưa camera về đúng target/distance/azimuth/elevation lúc khởi tạo
        void Reset();

        /*--- Override các phương thức từ ICamera ---*/

        // Lấy Ma trận Quan sát (View Matrix) - tự động tính lại nếu dữ liệu bị "dirty"
        const glm::mat4& GetViewMatrix() const override;

        // Lấy Ma trận Chiếu (Projection Matrix) - tự động tính lại nếu dữ liệu bị "dirty"
        const glm::mat4& GetProjectionMatrix() const override;

        // Lấy vị trí hiện tại của Camera trong World Space (C)
        glm::vec3 GetPosition() const override;

        // Lấy Vector hướng nhìn (Forward Vector / N)
        glm::vec3 GetForward() const override;

        // Lấy Vector hướng lên (Up Vector / V)
        glm::vec3 GetUp() const override;

        // Lấy Vector hướng sang phải (Right Vector / U)
        glm::vec3 GetRight() const override;

        const glm::vec3& GetTarget() const { return m_target; }

        float GetDistance() const { return m_distance; }

        float GetAzimuth() const { return m_azimuth; }

        float GetElevation() const { return m_elevation; }

        float GetFovY() const { return m_fovY; }

        // Chiều cao viewport (pixel), dùng để đổi pixel chuột -> đơn vị world khi pan
        float GetViewportHeight() const { return m_viewportHeight; }

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
        glm::vec3 m_target{ 0.0f, 0.0f, 0.0f };
        float m_distance{ 5.0f };   // Khoảng cách từ Camera đến Target
        float m_azimuth = 0.0f;     // Góc phương vị, quanh trục Y (yaw), luôn được wrap về [-π, π]
        float m_elevation = 0.0f;   // Góc nâng lên/xuống (pitch), clamp trong (-90°, 90°) để forward không song song world-up
        // (nếu song song thì cross(forward, worldUp) = 0 và Right bị suy biến)

// --- Giá trị khởi tạo, dùng cho Reset() ---
        glm::vec3 m_initialTarget{ 0.0f, 0.0f, 0.0f };
        float m_initialDistance{ 5.0f };
        float m_initialAzimuth = 0.0f;
        float m_initialElevation = 0.0f;

        /*--- Thuộc tính phép chiếu (Projection) ---*/

        // Góc mở chiều dọc (FOV Y)
        float m_fovY = glm::radians(45.0f);
        float m_near = 0.1f, m_far = 1000.0f; // Mặt phẳng cắt gần/xa
        float m_aspect = 16.0f / 9.0f;        // Tỷ lệ khung hình (Width / Height)
        float m_viewportHeight = 1080.0f;     // Chiều cao viewport (pixel)

        // --- Caching & Lazy Evaluation (mutable để cập nhật được trong các hàm const) ---
        mutable bool m_viewDirty = true;       // Cờ đánh dấu View Matrix cần tính toán lại
        mutable bool m_projectionDirty = true; // Cờ đánh dấu Projection Matrix cần tính toán lại

        mutable glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };  // Vị trí camera trong World Space (C)
        mutable glm::vec3 m_forward{ 0.0f, 0.0f, -1.0f };  // Hướng nhìn (Forward Vector / N), đã normalize
        mutable glm::vec3 m_up{ 0.0f, 1.0f, 0.0f };        // Vector hướng lên (Up Vector / V), đã normalize
        mutable glm::vec3 m_right{ 1.0f, 0.0f, 0.0f };     // Vector hướng sang phải (Right Vector / U), đã normalize

        mutable glm::mat4 m_viewMatrix{ 1.0f };       // Ma trận Quan sát (World -> View Space)
        mutable glm::mat4 m_projectionMatrix{ 1.0f }; // Ma trận Chiếu (View -> Clip Space)
    };
} // namespace Domain::Camera::OrbitCamera