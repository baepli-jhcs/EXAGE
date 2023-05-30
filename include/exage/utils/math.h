#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include "exage/Scene/Hierarchy.h"

namespace exage
{
    inline auto calculateTransformMatrix(Transform3D transform) noexcept -> glm::mat4
    {
        glm::mat4 scaleMatrix = glm::scale(transform.scale);

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        if (glm::quat* quat = std::get_if<glm::quat>(&transform.rotation); quat != nullptr)
        {
            rotationMatrix = glm::toMat4(*quat);
        }
        else if (glm::vec3* vec = std::get_if<glm::vec3>(&transform.rotation); vec != nullptr)
        {
            rotationMatrix = glm::toMat4(glm::quat {*vec});
        }

        glm::mat4 translationMatrix = glm::translate(transform.position);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    inline auto getForwardVector(glm::quat rotation, glm::vec3 forward = glm::vec3(0, 0, 1))
        -> glm::vec3
    {
        return glm::inverse(rotation) * forward;
    }

    inline auto getRightVector(glm::quat rotation, glm::vec3 right = glm::vec3(1, 0, 0))
        -> glm::vec3
    {
        return glm::inverse(rotation) * right;
    }

    inline auto getUpVector(glm::quat rotation, glm::vec3 up = glm::vec3(0, 1, 0)) -> glm::vec3
    {
        return glm::inverse(rotation) * up;
    }

    inline auto getViewMatrix(glm::vec3 position, glm::quat rotation) -> glm::mat4
    {
        glm::vec3 forward = getForwardVector(rotation);
        return glm::lookAt(
            position, position + forward, getUpVector(rotation, glm::vec3 {0, -1, 0}));
    }

    inline auto getViewMatrix(glm::vec3 position, glm::vec3 rotation) -> glm::mat4
    {
        glm::mat3 rotationMatrix = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
        return getViewMatrix(position, glm::quat(rotationMatrix));
    }
}  // namespace exage
