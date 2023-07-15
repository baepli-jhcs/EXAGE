#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"

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
                                               bool depthZeroToOne = true) noexcept -> Ray
    {
        glm::vec4 rayStart = glm::vec4(screenPointNDC.x, screenPointNDC.y, -1.0f, 1.0f);
        glm::vec4 rayEnd = glm::vec4(screenPointNDC.x, screenPointNDC.y, 0.0f, 1.0f);

        if (depthZeroToOne)
        {
            rayStart.z = 0.0f;
            rayEnd.z = 1.0f;
        }

        glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
        glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);

        glm::vec4 rayStartCamera = inverseProjectionMatrix * rayStart;
        rayStartCamera /= rayStartCamera.w;

        glm::vec4 rayStartWorld = inverseViewMatrix * rayStartCamera;
        rayStartWorld /= rayStartWorld.w;

        glm::vec4 rayEndCamera = inverseProjectionMatrix * rayEnd;
        rayEndCamera /= rayEndCamera.w;

        glm::vec4 rayEndWorld = inverseViewMatrix * rayEndCamera;
        rayEndWorld /= rayEndWorld.w;

        glm::vec3 rayDirectionWorld = glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));

        return {glm::vec3(rayStartWorld), rayDirectionWorld};
    }

};  // namespace exitor