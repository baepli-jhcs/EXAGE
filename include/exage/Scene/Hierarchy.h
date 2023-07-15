#pragma once

#include <iostream>
#include <variant>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include "exage/Scene/Entity.h"
#include "exage/Scene/Rotation3D.h"
#include "exage/utils/math.h"

namespace exage
{
    struct EntityRelationship
    {
        Entity parent = entt::null;

        uint32_t childCount = 0;
        Entity firstChild = entt::null;

        Entity nextSibling = entt::null;
        Entity previousSibling = entt::null;
    };

    struct RootEntity
    {
    };  // This is just a tag

    struct Transform3D
    {
        glm::vec3 position {0.F};
        glm::vec3 scale {1.F};
        Rotation3D rotation {};

        // Not intended to be modified by the user
        glm::mat4 matrix;

        glm::vec3 globalPosition;
        glm::vec3 globalScale;
        Rotation3D globalRotation;
        glm::mat4 globalMatrix;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(position, scale, rotation);
        }
    };

    struct EulerRotation
    {
        glm::vec3 euler;
    };

    inline auto calculateTransformMatrix(Transform3D transform) noexcept -> glm::mat4
    {
        glm::mat4 scaleMatrix = glm::scale(transform.scale);
        glm::mat4 rotationMatrix = transform.rotation.getRotationMatrix();
        glm::mat4 translationMatrix = glm::translate(transform.position);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }
}  // namespace exage
