#pragma once

#include <glm/glm.hpp>

namespace Model {

    // Interface tổng quát cho Bounding Box, dùng template để hỗ trợ
    // cả 2D (Dim = 2) lẫn 3D (Dim = 3) mà không lặp code.
    // Point dùng glm::vec<Dim, T> để tương thích trực tiếp với OpenGL
    // (glUniform, glVertexAttribPointer, các phép toán vector của glm, v.v.)
    template <typename T, int Dim>
    class IBoundingBox {
    public:
        using Point = glm::vec<Dim, T>;

        virtual ~IBoundingBox() = default;

        // --- Truy vấn thông tin cơ bản ---

        virtual Point getMin() const = 0;
        virtual Point getMax() const = 0;
        virtual Point getCenter() const = 0;
        virtual Point getSize() const = 0;

        // --- Kiểm tra quan hệ hình học ---

        virtual bool contains(const Point& point) const = 0;
        virtual bool intersects(const IBoundingBox<T, Dim>& other) const = 0;

        // --- Cập nhật / mở rộng box ---

        virtual void expand(const Point& point) = 0;
        virtual void expand(const IBoundingBox<T, Dim>& other) = 0;
        virtual void reset() = 0; // đưa box về trạng thái rỗng/invalid

        // --- Thông tin phụ trợ ---

        virtual T volume() const = 0; // thể tích nếu Dim = 3
        virtual bool isValid() const = 0; // false nếu box chưa chứa điểm nào
    };

    // Alias tiện dụng cho các trường hợp phổ biến
    // (Point tương ứng sẽ là glm::vec2 / glm::vec3 khi T = float)
    template <typename T>
    using IBoundingBox2D = IBoundingBox<T, 2>;

    template <typename T>
    using IBoundingBox3D = IBoundingBox<T, 3>;

} // namespace Model