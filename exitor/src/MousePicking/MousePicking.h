#pragma once

#include "Ray.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Scene.h"

namespace exitor
{
    [[nodiscard]] inline auto testRayAABBIntersection(Ray ray,
                                                      exage::Renderer::AABB aabb,
                                                      glm::vec3 position,
                                                      glm::mat4 modelMatrix)
        -> std::pair<bool, float>
    {
        constexpr float MAX_INTERSECTION_DISTANCE = 100000.0F;
        constexpr float EPSILON = 0.000001F;

        float tMin = 0.0F;
        float tMax = MAX_INTERSECTION_DISTANCE;

        glm::vec3 delta = position - ray.origin;

        auto testIntersectionWithAxis = [&](uint8_t axis) -> bool
        {
            glm::vec3 axisVector = glm::vec3(modelMatrix[axis]);
            float e = glm::dot(axisVector, delta);
            float f = glm::dot(ray.direction, axisVector);

            if (std::abs(f) > EPSILON)
            {
                float t1 = (e + aabb.min[axis]) / f;
                float t2 = (e + aabb.max[axis]) / f;

                if (t1 > t2)
                {
                    std::swap(t1, t2);
                }

                if (t1 > tMin)
                {
                    tMin = t1;
                }

                if (t2 < tMax)
                {
                    tMax = t2;
                }

                if (tMin > tMax)
                {
                    return false;
                }

                if (tMax < 0.0F)
                {
                    return false;
                }
            }
            else if (-e + aabb.min[axis] > 0.0F || -e + aabb.max[axis] < 0.0F)
            {
                return false;
            }

            return true;
        };

        if (!testIntersectionWithAxis(0))
        {
            return {false, 0.0F};
        }

        if (!testIntersectionWithAxis(1))
        {
            return {false, 0.0F};
        }

        if (!testIntersectionWithAxis(2))
        {
            return {false, 0.0F};
        }

        return {true, tMin};
    }

    [[nodiscard]] inline auto getSelectedEntity(exage::Scene& scene,
                                                const glm::mat4& viewMatrix,
                                                const glm::mat4& projectionMatrix,
                                                glm::vec2 screenPointNDC) noexcept -> exage::Entity
    {
        Ray ray = screenPointToRay(screenPointNDC, viewMatrix, projectionMatrix);

        exage::Entity selectedEntity = entt::null;
        float minDistance = std::numeric_limits<float>::max();

        entt::registry& reg = scene.registry();

        auto view = reg.view<exage::Renderer::StaticMesh, exage::Transform3D>();

        for (auto entity : view)
        {
            exage::Renderer::StaticMesh& mesh = view.get<exage::Renderer::StaticMesh>(entity);
            exage::Transform3D& transform = view.get<exage::Transform3D>(entity);

            auto [intersect, distance] =
                testRayAABBIntersection(ray, mesh.aabb, transform.position, transform.globalMatrix);

            if (intersect && distance < minDistance)
            {
                minDistance = distance;
                selectedEntity = entity;
            }
        }

        return selectedEntity;
    }
}  // namespace exitor