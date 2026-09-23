#pragma once

// Header-only helper: đọc file model bằng Assimp -> tính bounding box (AABB)
// -> tự tính camera (view/projection/near/far) để thấy trọn model.
// Không phụ thuộc vào Model::RenderModel hay engine, chỉ cần Assimp + GLM.

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

namespace ModelUtils {

    // ---------------------------------------------------------------------------
    // Bounding box
    // ---------------------------------------------------------------------------
    struct BoundingBox {
        glm::vec3 Min{ std::numeric_limits<float>::max() };
        glm::vec3 Max{ std::numeric_limits<float>::lowest() };
        bool Valid = false;

        void Expand(const glm::vec3& p) {
            Min = glm::min(Min, p);
            Max = glm::max(Max, p);
            Valid = true;
        }

        glm::vec3 Center() const { return (Min + Max) * 0.5f; }
        glm::vec3 Size() const { return Max - Min; }
        // Bán kính của hình cầu bao quanh AABB
        float Radius() const { return glm::length(Size()) * 0.5f; }
    };

    namespace Detail {

        inline glm::mat4 ToGlm(const aiMatrix4x4& m) {
            // Assimp là row-major, GLM là column-major -> transpose
            return glm::mat4(
                m.a1, m.b1, m.c1, m.d1,
                m.a2, m.b2, m.c2, m.d2,
                m.a3, m.b3, m.c3, m.d3,
                m.a4, m.b4, m.c4, m.d4);
        }

        inline void Accumulate(const aiScene* scene, const aiNode* node,
            const glm::mat4& parent, bool applyNodeTransforms,
            BoundingBox& box) {
            const glm::mat4 world =
                applyNodeTransforms ? parent * ToGlm(node->mTransformation) : glm::mat4(1.0f);

            for (unsigned i = 0; i < node->mNumMeshes; ++i) {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                for (unsigned v = 0; v < mesh->mNumVertices; ++v) {
                    const aiVector3D& p = mesh->mVertices[v];
                    box.Expand(glm::vec3(world * glm::vec4(p.x, p.y, p.z, 1.0f)));
                }
            }
            for (unsigned c = 0; c < node->mNumChildren; ++c) {
                Accumulate(scene, node->mChildren[c], world, applyNodeTransforms, box);
            }
        }

    } // namespace Detail

    // Tính AABB của toàn bộ model trong file.
    // applyNodeTransforms = true : áp dụng transform của node hierarchy (đúng với
    //                              hầu hết importer). Đặt false nếu importer của
    //                              engine bỏ qua node transform.
    // Trả về box.Valid == false nếu đọc file lỗi hoặc không có vertex.
    inline BoundingBox ComputeBoundingBox(const std::string& path,
        bool applyNodeTransforms = true) {
        BoundingBox box;

        Assimp::Importer importer;
        // flags = 0: chỉ cần đọc vị trí vertex, không cần triangulate/join... nên nhanh
        const aiScene* scene = importer.ReadFile(path, 0);
        if (!scene || !scene->mRootNode) {
            return box;
        }

        Detail::Accumulate(scene, scene->mRootNode, glm::mat4(1.0f), applyNodeTransforms, box);
        return box;
    }

    // ---------------------------------------------------------------------------
    // Camera fit
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
    // Model matrix có thể để identity (không cần dời model về gốc).
    inline CameraSetup FitCameraToBox(const BoundingBox& box,
        float fovYDeg = 45.0f,
        float aspect = 1.0f,
        float yawDeg = 0.0f,
        float pitchDeg = 0.0f,
        float margin = 1.1f) {
        CameraSetup cam;
        if (!box.Valid) {
            return cam;
        }

        const glm::vec3 center = box.Center();
        const float radius = std::max(box.Radius(), 1e-6f);

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

} // namespace ModelUtils