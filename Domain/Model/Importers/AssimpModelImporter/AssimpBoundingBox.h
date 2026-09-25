#pragma once

// Cài đặt IBoundingBox3D<float> bằng dữ liệu đọc từ file model qua Assimp.
// Chỉ phụ thuộc Assimp + GLM + IBoundingBox.hpp, không phụ thuộc engine.

#include <Model/IBoundingBox.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

using BoundingBox3D = Model::IBoundingBox3D<float>;

namespace Model {
    // Bounding box (AABB) cài đặt từ IBoundingBox3D<float>, dữ liệu lấy từ Assimp.
    class AssimpBoundingBox final : public BoundingBox3D {
    public:
        AssimpBoundingBox() { reset(); }

        // Đọc file bằng Assimp và tính AABB bao toàn bộ model.
        // applyNodeTransforms = true : áp dụng transform của node hierarchy (đúng với
        //                              hầu hết importer). Đặt false nếu importer của
        //                              engine bỏ qua node transform.
        // Trả về box với isValid() == false nếu đọc file lỗi hoặc không có vertex.
        static AssimpBoundingBox FromFile(const std::string& path,
            bool applyNodeTransforms = true) {
            AssimpBoundingBox box;

            Assimp::Importer importer;
            // flags = 0: chỉ cần đọc vị trí vertex, không cần triangulate/join... nên nhanh
            const aiScene* scene = importer.ReadFile(path, 0);
            if (!scene || !scene->mRootNode) {
                return box;
            }

            accumulate(scene, scene->mRootNode, glm::mat4(1.0f), applyNodeTransforms, box);
            return box;
        }

        // --- IBoundingBox3D<float> ---
        Point getMin() const override { return m_min; }
        Point getMax() const override { return m_max; }
        Point getCenter() const override { return (m_min + m_max) * 0.5f; }
        Point getSize() const override { return m_max - m_min; }

        bool contains(const Point& point) const override {
            return m_valid &&
                point.x >= m_min.x && point.x <= m_max.x &&
                point.y >= m_min.y && point.y <= m_max.y &&
                point.z >= m_min.z && point.z <= m_max.z;
        }

        bool intersects(const BoundingBox3D& other) const override {
            if (!m_valid || !other.isValid()) return false;
            const Point oMin = other.getMin();
            const Point oMax = other.getMax();
            return  m_min.x <= oMax.x && m_max.x >= oMin.x &&
                    m_min.y <= oMax.y && m_max.y >= oMin.y &&
                    m_min.z <= oMax.z && m_max.z >= oMin.z;
        }

        void expand(const Point& point) override {
            m_min = glm::min(m_min, point);
            m_max = glm::max(m_max, point);
            m_valid = true;
        }

        void expand(const Model::IBoundingBox<float, 3>& other) override {
            if (!other.isValid()) return;
            expand(other.getMin());
            expand(other.getMax());
        }

        void reset() override {
            m_min = Point(std::numeric_limits<float>::max());
            m_max = Point(std::numeric_limits<float>::lowest());
            m_valid = false;
        }

        float volume() const override {
            if (!m_valid) return 0.0f;
            const Point s = getSize();
            return s.x * s.y * s.z;
        }

        bool isValid() const override { return m_valid; }

        // Bán kính hình cầu bao quanh AABB (tiện cho việc fit camera)
        float radius() const { return glm::length(getSize()) * 0.5f; }

    private:
        Point m_min{ 0.0f };
        Point m_max{ 0.0f };
        bool  m_valid = false;

        static glm::mat4 toGlm(const aiMatrix4x4& m) {
            // Assimp là row-major, GLM là column-major -> transpose
            return glm::mat4(
                m.a1, m.b1, m.c1, m.d1,
                m.a2, m.b2, m.c2, m.d2,
                m.a3, m.b3, m.c3, m.d3,
                m.a4, m.b4, m.c4, m.d4);
        }

        static void accumulate(const aiScene* scene, const aiNode* node,
            const glm::mat4& parent, bool applyNodeTransforms,
            AssimpBoundingBox& box) {
            const glm::mat4 world =
                applyNodeTransforms ? parent * toGlm(node->mTransformation) : glm::mat4(1.0f);

            for (unsigned i = 0; i < node->mNumMeshes; ++i) {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                for (unsigned v = 0; v < mesh->mNumVertices; ++v) {
                    const aiVector3D& p = mesh->mVertices[v];
                    box.expand(glm::vec3(world * glm::vec4(p.x, p.y, p.z, 1.0f)));
                }
            }
            for (unsigned c = 0; c < node->mNumChildren; ++c) {
                accumulate(scene, node->mChildren[c], world, applyNodeTransforms, box);
            }
        }
    };

    // ---------------------------------------------------------------------------
    // Camera fit — nhận bất kỳ IBoundingBox3D<float> nào, không riêng AssimpBoundingBox
    // ---------------------------------------------------------------------------
    struct CameraSetup {
        glm::vec3 Position{ 0.0f };
        glm::vec3 Target{ 0.0f };
        float NearPlane = 0.1f;
        float FarPlane = 100.0f;
        glm::mat4 View{ 1.0f };
        glm::mat4 Projection{ 1.0f };

        glm::mat4 ViewProjection() const { return Projection * View; }
    };

    // Đặt camera nhìn vào tâm box, đủ xa để hình cầu bao quanh model lọt khung hình.
    //   yawDeg   : xoay quanh trục Y.  0 = nhìn từ phía +Z, 90 = từ phía +X
    //   pitchDeg : góc nâng.           0 = ngang, dương = nhìn từ trên xuống
    //   margin   : hệ số chừa lề (1.1 = 10%)
    inline CameraSetup FitCameraToBox(const BoundingBox3D& box,
        float fovYDeg = 45.0f,
        float aspect = 1.0f,
        float yawDeg = 0.0f,
        float pitchDeg = 0.0f,
        float margin = 1.1f) {
        CameraSetup cam;
        if (!box.isValid()) {
            return cam;
        }

        const glm::vec3 center = box.getCenter();
        const glm::vec3 size = box.getSize();
        const float radius = std::max(glm::length(size) * 0.5f, 1e-6f);

        // Khung hình bị giới hạn bởi FOV nhỏ hơn giữa chiều dọc và chiều ngang
        const float fovY = glm::radians(fovYDeg);
        const float fovX = 2.0f * std::atan(std::tan(fovY * 0.5f) * aspect);
        const float limitingFov = std::min(fovY, fovX);

        const float distance = radius / std::sin(limitingFov * 0.5f) * margin;

        const float yaw = glm::radians(yawDeg);
        const float pitch = glm::radians(pitchDeg);
        const glm::vec3 dir(std::cos(pitch) * std::sin(yaw),
            std::sin(pitch),
            std::cos(pitch) * std::cos(yaw));

        // Tránh degenerate khi nhìn thẳng lên/xuống (up song song với hướng nhìn)
        const glm::vec3 up = (std::abs(dir.y) > 0.999f) ? glm::vec3(0, 0, -1) : glm::vec3(0, 1, 0);

        cam.Target = center;
        cam.Position = center + dir * distance;
        cam.NearPlane = std::max(radius * 0.01f, distance - radius);
        cam.FarPlane = distance + radius;
        cam.View = glm::lookAt(cam.Position, cam.Target, up);
        cam.Projection = glm::perspective(fovY, aspect, cam.NearPlane, cam.FarPlane);
        return cam;
    }
};