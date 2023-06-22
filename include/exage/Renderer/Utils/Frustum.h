#pragma once

#include <array>

#include <glm/glm.hpp>

#include "exage/Renderer/Scene/Mesh.h"

namespace exage::Renderer
{
    struct Frustum
    {
        std::array<glm::vec4, 6> planes {};

        Frustum() = default;
        explicit Frustum(const glm::mat4& modelViewProjection) noexcept
        {
            planes[0] = glm::vec4(modelViewProjection[0][3] + modelViewProjection[0][0],
                                  modelViewProjection[1][3] + modelViewProjection[1][0],
                                  modelViewProjection[2][3] + modelViewProjection[2][0],
                                  modelViewProjection[3][3] + modelViewProjection[3][0]);

            planes[1] = glm::vec4(modelViewProjection[0][3] - modelViewProjection[0][0],
                                  modelViewProjection[1][3] - modelViewProjection[1][0],
                                  modelViewProjection[2][3] - modelViewProjection[2][0],
                                  modelViewProjection[3][3] - modelViewProjection[3][0]);

            planes[2] = glm::vec4(modelViewProjection[0][3] + modelViewProjection[0][1],
                                  modelViewProjection[1][3] + modelViewProjection[1][1],
                                  modelViewProjection[2][3] + modelViewProjection[2][1],
                                  modelViewProjection[3][3] + modelViewProjection[3][1]);

            planes[3] = glm::vec4(modelViewProjection[0][3] - modelViewProjection[0][1],
                                  modelViewProjection[1][3] - modelViewProjection[1][1],
                                  modelViewProjection[2][3] - modelViewProjection[2][1],
                                  modelViewProjection[3][3] - modelViewProjection[3][1]);

            planes[4] = glm::vec4(modelViewProjection[0][3] + modelViewProjection[0][2],
                                  modelViewProjection[1][3] + modelViewProjection[1][2],
                                  modelViewProjection[2][3] + modelViewProjection[2][2],
                                  modelViewProjection[3][3] + modelViewProjection[3][2]);

            planes[5] = glm::vec4(modelViewProjection[0][3] - modelViewProjection[0][2],
                                  modelViewProjection[1][3] - modelViewProjection[1][2],
                                  modelViewProjection[2][3] - modelViewProjection[2][2],
                                  modelViewProjection[3][3] - modelViewProjection[3][2]);

            for (auto& plane : planes)
            {
                plane = glm::normalize(plane);
            }
        }

        [[nodiscard]] auto intersects(const AABB& aabb) const noexcept -> bool
        {
            for (const auto& plane : planes)
            {
                glm::vec3 positiveVertex = aabb.min;
                if (plane.x >= 0.0f)
                {
                    positiveVertex.x = aabb.max.x;
                }
                if (plane.y >= 0.0f)
                {
                    positiveVertex.y = aabb.max.y;
                }
                if (plane.z >= 0.0f)
                {
                    positiveVertex.z = aabb.max.z;
                }

                if (glm::dot(plane, glm::vec4(positiveVertex, 1.0f)) < 0.0f)
                {
                    return false;
                }
            }

            return true;
        }
    };

    inline auto getFrustumCorners(bool depthZeroToOne) noexcept -> std::array<glm::vec3, 8>
    {
        if (depthZeroToOne)
        {
            return {
                glm::vec3(-1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(-1.0f, 1.0f, 1.0f),
            };
        }
        else
        {
            return {
                glm::vec3(-1.0f, -1.0f, -1.0f),
                glm::vec3(1.0f, -1.0f, -1.0f),
                glm::vec3(1.0f, 1.0f, -1.0f),
                glm::vec3(-1.0f, 1.0f, -1.0f),
                glm::vec3(-1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(-1.0f, 1.0f, 1.0f),
            };
        }
    }
}  // namespace exage::Renderer