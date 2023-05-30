#pragma once

#include <iostream>
#include <variant>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "exage/Scene/Entity.h"

namespace exage
{
    struct EntityRelationship
    {
        Entity parent;

        size_t childCount;
        Entity firstChild;

        Entity nextSibling;
        Entity previousSibling;
    };

    struct RootEntity
    {
    };  // This is just a tag

    using Rotation3D = std::variant<glm::vec3, glm::quat>;

    struct Transform3D
    {
        glm::vec3 position {0.F};
        glm::vec3 scale {1.F};
        Rotation3D rotation {glm::identity<glm::quat>()};
        glm::mat4 matrix;

        // Not intended to be modified by the user
        glm::vec3 globalPosition;
        glm::vec3 globalScale;
        glm::quat globalRotation;
        glm::mat4 globalMatrix;

        [[nodiscard]] auto getQuatRotation() const noexcept -> glm::quat
        {
            if (const glm::quat* quat = std::get_if<glm::quat>(&rotation); quat != nullptr)
            {
                return *quat;
            }
            else if (const glm::vec3* vec = std::get_if<glm::vec3>(&rotation); vec != nullptr)
            {
                return glm::quat {*vec};
            }
            else
            {
                return glm::quat {};
            }
        }
    };
}  // namespace exage
