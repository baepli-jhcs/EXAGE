#pragma once

#include "exage/Core/Core.h"
#include "exage/Scene/Rotation3D.h"
#include "exage/utils/math.h"
#include "glm/glm.hpp"

namespace exitor
{
    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;

        [[nodiscard]] auto at(float t) const noexcept -> glm::vec3
        {
            return origin + t * direction;
        }
    };

    [[nodiscard]] inline auto screenPointToRay(glm::vec2 screenPointNDC,
                                               const glm::mat4& viewMatrix,
                                               const glm::mat4& projectionMatrix,
                                               glm::vec3 position) noexcept -> Ray
    {
        // glm::vec4 rayStart = glm::vec4(screenPointNDC.x, screenPointNDC.y, -1.0f, 1.0f);
        // glm::vec4 rayEnd = glm::vec4(screenPointNDC.x, screenPointNDC.y, 0.0f, 1.0f);

        // if (depthZeroToOne)
        // {
        //     rayStart.z = 0.0f;
        //     rayEnd.z = 1.0f;
        // }

        // glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
        // glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);

        // glm::vec4 rayStartCamera = inverseProjectionMatrix * rayStart;
        // rayStartCamera /= rayStartCamera.w;

        // glm::vec4 rayStartWorld = inverseViewMatrix * rayStartCamera;
        // rayStartWorld /= rayStartWorld.w;

        // glm::vec4 rayEndCamera = inverseProjectionMatrix * rayEnd;
        // rayEndCamera /= rayEndCamera.w;

        // glm::vec4 rayEndWorld = inverseViewMatrix * rayEndCamera;
        // rayEndWorld /= rayEndWorld.w;

        // glm::vec3 rayDirectionWorld = glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));

        // return {glm::vec3(rayStartWorld), rayDirectionWorld};

        // glm::vec4 rayClip = glm::vec4(screenPointNDC.x, screenPointNDC.y, -1.0f, 1.0f);
        // if (depthZeroToOne)
        // {
        //     rayClip.z = 0.0f;
        // }

        // glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);

        // glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
        // rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        // glm::vec4 rayWorld = inverseViewMatrix * rayEye;
        // glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

        // glm::vec3 rayOrigin = glm::vec3(inverseViewMatrix[3]);

        // return {rayOrigin, rayDirection};

        glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
        glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);

        glm::vec4 rayStart = glm::vec4(screenPointNDC.x, screenPointNDC.y, exage::Z_AXIS.z, 1.0f);

        glm::vec4 rayStartCamera = inverseProjectionMatrix * rayStart;
        rayStartCamera /= rayStartCamera.w;

        glm::vec4 rayStartWorld = inverseViewMatrix * rayStartCamera;
        rayStartWorld /= rayStartWorld.w;

        glm::vec3 rayDirectionWorld = glm::normalize(glm::vec3(rayStartWorld) - position);

        return {position, rayDirectionWorld};
    }

};  // namespace exitor