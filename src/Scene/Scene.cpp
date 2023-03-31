#include "exage/Scene/Scene.h"

#include "exage/Scene/Entity.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/utils/math.h"

namespace exage
{
    auto Scene::createEntity(Entity parent) noexcept -> Entity
    {
        Entity const entity {_registry.create(), *this};
        auto& relationship = entity.addComponent<EntityRelationship>();

        relationship.parent = parent;

        if (parent.isValid())
        {
            auto& parentRelationship = parent.getComponent<EntityRelationship>();

            if (parentRelationship.firstChild.isValid())
            {
                auto& firstChildRelationship =
                    parentRelationship.firstChild.getComponent<EntityRelationship>();

                firstChildRelationship.previousSibling = entity;
                relationship.nextSibling = parentRelationship.firstChild;
            }

            parentRelationship.firstChild = entity;
            parentRelationship.childCount++;
        }

        return {_registry.create(), *this};
    }

    void Scene::destroyEntity(Entity& entity) noexcept
    {
        auto& relationship = entity.getComponent<EntityRelationship>();

        if (relationship.parent.isValid())
        {
            auto& parentRelationship = relationship.parent.getComponent<EntityRelationship>();
            parentRelationship.childCount--;
            if (parentRelationship.firstChild == entity)
            {
                parentRelationship.firstChild = relationship.nextSibling;
            }
        }

        if (relationship.nextSibling.isValid())
        {
            auto& nextSiblingRelationship =
                relationship.nextSibling.getComponent<EntityRelationship>();
            nextSiblingRelationship.previousSibling = relationship.previousSibling;
        }

        if (relationship.previousSibling.isValid())
        {
            auto& previousSiblingRelationship =
                relationship.previousSibling.getComponent<EntityRelationship>();
            previousSiblingRelationship.nextSibling = relationship.nextSibling;
        }

        size_t const childCount = relationship.childCount;
        for (size_t i = 0; i < childCount; i++)
        {
            destroyEntity(relationship.firstChild);
        }

        _registry.destroy(entity.getHandle());
        entity = {};
    }

    static void calculateChildTransform(glm::mat4 parentTransform, Entity entity) noexcept
    {
        auto& childTransform = entity.getComponent<Transform3D>();
        childTransform.localMatrix = calculateTransformMatrix(childTransform);
        childTransform.globalMatrix = parentTransform * childTransform.localMatrix;
        entity.forEachChild([&](Entity child)
                            { calculateChildTransform(childTransform.globalMatrix, child); });
    }

    void Scene::updateHierarchy(bool calculateTransforms) noexcept
    {
        auto view = _registry.view<EntityRelationship>();
        for (auto entity : view)
        {
            auto& relationship = view.get<EntityRelationship>(entity);

            if (!relationship.parent.isValid())
            {
                _registry.emplace_or_replace<RootEntity>(entity);
            }
            else
            {
                _registry.remove<RootEntity>(entity);
            }  // All entities that have a parent are not root entities
        }

        if (calculateTransforms)
        {
            auto view = _registry.view<EntityRelationship, Transform3D, RootEntity>();
            for (auto entity : view)
            {
                auto& relationship = view.get<EntityRelationship>(entity);
                auto& transform = view.get<Transform3D>(entity);
                transform.localMatrix = calculateTransformMatrix(transform);
                transform.globalMatrix = transform.localMatrix;

                Entity entt = {entity, *this};

                entt.forEachChild([&](Entity child)
                                  { calculateChildTransform(transform.globalMatrix, child); });
            }
        }
    }
}  // namespace exage
