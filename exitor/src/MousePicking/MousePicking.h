#pragma once

#include <float.h>

#include "Ray.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Camera.h"
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
        // constexpr float MAX_INTERSECTION_DISTANCE = 100000.0F;
        // constexpr float EPSILON = 0.000001F;

        // float tMin = 0.0F;
        // float tMax = MAX_INTERSECTION_DISTANCE;

        // glm::vec3 delta = position - ray.origin;

        // auto testIntersectionWithAxis = [&](uint8_t axis) -> bool
        // {
        //     glm::vec3 axisVector =
        //         glm::vec3(modelMatrix[axis].x, modelMatrix[axis].y, modelMatrix[axis].z);
        //     float e = glm::dot(axisVector, delta);
        //     float f = glm::dot(ray.direction, axisVector);

        //     if (std::abs(f) > EPSILON)
        //     {
        //         float t1 = (e + aabb.min[axis]) / f;
        //         float t2 = (e + aabb.max[axis]) / f;

        //         if (t1 > t2)
        //         {
        //             std::swap(t1, t2);
        //         }

        //         if (t2 < tMax)
        //         {
        //             tMax = t2;
        //         }

        //         if (t1 > tMin)
        //         {
        //             tMin = t1;
        //         }

        //         if (tMin > tMax)
        //         {
        //             return false;
        //         }
        //     }
        //     else if (-e + aabb.min[axis] > 0.0F || -e + aabb.max[axis] < 0.0F)
        //     {
        //         return false;
        //     }

        //     return true;
        // };

        // if (!testIntersectionWithAxis(0))
        // {
        //     return {false, 0.0F};
        // }

        // if (!testIntersectionWithAxis(1))
        // {
        //     return {false, 0.0F};
        // }

        // if (!testIntersectionWithAxis(2))
        // {
        //     return {false, 0.0F};
        // }

        // return {true, tMin};

        glm::mat4 inverseModelMatrix = glm::inverse(modelMatrix);

        ray.origin = glm::vec3(inverseModelMatrix * glm::vec4(ray.origin, 1.0F));
        ray.direction = glm::vec3(inverseModelMatrix * glm::vec4(ray.direction, 0.0F));

        float tMin = -std::numeric_limits<float>::infinity();
        float tMax = std::numeric_limits<float>::infinity();

        for (uint8_t i = 0; i < 3; i++)
        {
            if (std::abs(ray.direction[i]) > glm::epsilon<float>())
            {
                float t1 = (aabb.min[i] - ray.origin[i]) / ray.direction[i];
                float t2 = (aabb.max[i] - ray.origin[i]) / ray.direction[i];

                tMin = std::max(tMin, std::min(t1, t2));
                tMax = std::min(tMax, std::max(t1, t2));
            }
            else if (ray.origin[i] < aabb.min[i] || ray.origin[i] > aabb.max[i])
            {
                return {false, 0.0F};
            }
        }

        return {tMin <= tMax && tMax > 0.F, tMin};
    }

    [[nodiscard]] inline auto getSelectedEntity(exage::Renderer::AssetCache& assetCache,
                                                exage::Scene& scene,
                                                exage::Entity cameraEntity,
                                                const glm::mat4& viewMatrix,
                                                const glm::mat4& projectionMatrix,
                                                glm::vec2 screenPointNDC) noexcept -> exage::Entity
    {
        auto& cameraTransform = scene.registry().get<exage::Transform3D>(cameraEntity);

        Ray ray = screenPointToRay(
            screenPointNDC, viewMatrix, projectionMatrix, cameraTransform.globalPosition);
        std::cout << "Ray direction x, y, z: " << ray.direction.x << ", " << ray.direction.y << ", "
                  << ray.direction.z << std::endl;
        std::cout << "Ray origin x, y, z: " << ray.origin.x << ", " << ray.origin.y << ", "
                  << ray.origin.z << std::endl;

        exage::Entity selectedEntity = entt::null;
        float minDistance = std::numeric_limits<float>::max();

        entt::registry& reg = scene.registry();

        auto view = reg.view<exage::Renderer::StaticMeshComponent, exage::Transform3D>();

        for (auto entity : view)
        {
            exage::Renderer::StaticMeshComponent& meshComponent =
                view.get<exage::Renderer::StaticMeshComponent>(entity);
            exage::Transform3D& transform = view.get<exage::Transform3D>(entity);

            if (!assetCache.hasMesh(meshComponent.pathHash))
            {
                continue;
            }

            const exage::Renderer::GPUStaticMesh& mesh = assetCache.getMesh(meshComponent.pathHash);

            auto [intersect, distance] = testRayAABBIntersection(
                ray, mesh.aabb, transform.globalPosition, transform.globalMatrix);

            if (intersect && distance < minDistance)
            {
                minDistance = distance;
                selectedEntity = entity;
            }
        }

        return selectedEntity;
    }
}  // namespace exitor