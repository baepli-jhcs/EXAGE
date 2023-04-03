﻿#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "exage/Scene/Hierarchy.h"

namespace exage
{
    inline auto calculateTransformMatrix(Transform3D transform) noexcept -> glm::mat4
    {
        glm::mat4 scaleMatrix = glm::scale(transform.scale);

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        if (glm::quat* quat = std::get_if < glm::quat>(&transform.rotation); quat != nullptr)
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
}  // namespace exage