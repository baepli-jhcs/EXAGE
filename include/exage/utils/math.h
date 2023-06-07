#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace exage
{
    constexpr float SMALL_NUMBER = 0.00000001F;
    constexpr glm::vec3 X_AXIS = glm::vec3(1, 0, 0);
    constexpr glm::vec3 Y_AXIS = glm::vec3(0, 1, 0);
    constexpr glm::vec3 Z_AXIS = glm::vec3(0, 0, -1);

    inline auto getQuaternionRotation(glm::vec3 yawPitchRoll) -> glm::quat
    {
        glm::quat yaw = glm::angleAxis(yawPitchRoll.y, glm::vec3(0, 1, 0));
        glm::quat pitch = glm::angleAxis(yawPitchRoll.x, glm::vec3(1, 0, 0));
        glm::quat roll = glm::angleAxis(yawPitchRoll.z, glm::vec3(0, 0, 1));

        return yaw * pitch * roll;
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

    inline auto getViewMatrix(glm::vec3 position,
                              glm::quat rotation,
                              glm::vec3 up = glm::vec3(0, 1, 0)) -> glm::mat4
    {
        glm::vec3 forward = getForwardVector(rotation);
        return glm::lookAt(position, position + forward, getUpVector(rotation, up));
    }

    inline auto getViewMatrix(glm::vec3 position,
                              glm::vec3 yawPitchRoll,
                              glm::vec3 up = glm::vec3(0, 1, 0)) -> glm::mat4
    {
        glm::quat rotation = getQuaternionRotation(yawPitchRoll);
        return getViewMatrix(position, rotation, up);
    }
}  // namespace exage
