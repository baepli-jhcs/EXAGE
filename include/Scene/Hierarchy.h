#pragma once

#include <iostream>
#include <variant>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Scene/Entity.h"

namespace exage
{
    using Rotation3D = std::variant<glm::vec3, glm::quat>;

    struct Transform3D
    {
        glm::vec3 position;
        glm::vec3 scale;
        Rotation3D rotation;

        glm::mat4 localMatrix;
        glm::mat4 globalMatrix;
    };

    struct EntityRelationship
    {
        Entity parent;

        size_t childCount;
        Entity firstChild;

        Entity nextSibling;
        Entity previousSibling;
    };
}  // namespace exage
