#include "exage/Scene/Scene.h"

#include "exage/Scene/Entity.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/utils/math.h"

namespace exage
{
    auto Scene::createEntity(Entity parent) noexcept -> Entity
    {
        Entity entity = _registry.create();
        auto& relationship = addComponent<EntityRelationship>(entity);

        relationship.parent = parent;

        if (parent != entt::null)
        {
            auto& parentRelationship = getComponent<EntityRelationship>(parent);

            if (parentRelationship.firstChild != entt::null)
            {
                auto& firstChildRelationship =
                    getComponent<EntityRelationship>(parentRelationship.firstChild);

                firstChildRelationship.previousSibling = entity;
                relationship.nextSibling = parentRelationship.firstChild;
            }

            parentRelationship.firstChild = entity;
            parentRelationship.childCount++;
        }

        return entity;
    }

    void Scene::destroyEntity(Entity entity) noexcept
    {
        auto& relationship = getComponent<EntityRelationship>(entity);

        if (relationship.parent != entt::null)
        {
            auto& parentRelationship = getComponent<EntityRelationship>(relationship.parent);
            parentRelationship.childCount--;
            if (parentRelationship.firstChild == entity)
            {
                parentRelationship.firstChild = relationship.nextSibling;
            }
        }

        if (relationship.nextSibling != entt::null)
        {
            auto& nextSiblingRelationship =
                getComponent<EntityRelationship>(relationship.nextSibling);
            nextSiblingRelationship.previousSibling = relationship.previousSibling;
        }

        if (relationship.previousSibling != entt::null)
        {
            auto& previousSiblingRelationship =
                getComponent<EntityRelationship>(relationship.previousSibling);
            previousSiblingRelationship.nextSibling = relationship.nextSibling;
        }

        size_t const childCount = relationship.childCount;
        for (size_t i = 0; i < childCount; i++)
        {
            destroyEntity(relationship.firstChild);
        }

        _registry.destroy(entity);
    }

    void Scene::calculateChildTransform(Transform3D& parentTransform, Entity entity) noexcept
    {
        auto& childTransform = getComponent<Transform3D>(entity);
        glm::quat childRotation = childTransform.rotation.getQuaternion();

        childTransform.globalRotation = parentTransform.globalRotation * childRotation;
        childTransform.globalPosition = parentTransform.globalPosition
            + parentTransform.globalRotation.getQuaternion() * childTransform.position;
        childTransform.globalScale = parentTransform.globalScale * childTransform.scale;

        childTransform.matrix = calculateTransformMatrix(childTransform);
        childTransform.globalMatrix = parentTransform.globalMatrix * childTransform.matrix;

        forEachChild(entity, [&](Entity child) { calculateChildTransform(childTransform, child); });
    }

    void Scene::updateHierarchy(bool calculateTransforms) noexcept
    {
        auto view = _registry.view<EntityRelationship>();
        for (auto entity : view)
        {
            auto& relationship = view.get<EntityRelationship>(entity);

            if (relationship.parent == entt::null)
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
                auto& transform = view.get<Transform3D>(entity);
                transform.globalRotation = transform.rotation;
                transform.globalPosition = transform.position;
                transform.globalScale = transform.scale;
                transform.matrix = calculateTransformMatrix(transform);
                transform.globalMatrix = transform.matrix;

                forEachChild(entity,
                             [&](Entity child) { calculateChildTransform(transform, child); });
            }
        }
    }

    void Scene::setParent(Entity entity, Entity parent) noexcept
    {
        auto& relationship = getComponent<EntityRelationship>(entity);

        if (relationship.parent != entt::null)
        {
            auto& parentRelationship = getComponent<EntityRelationship>(relationship.parent);
            parentRelationship.childCount--;
            if (parentRelationship.firstChild == entity)
            {
                parentRelationship.firstChild = relationship.nextSibling;
            }
        }

        if (relationship.nextSibling != entt::null)
        {
            auto& nextSiblingRelationship =
                getComponent<EntityRelationship>(relationship.nextSibling);
            nextSiblingRelationship.previousSibling = relationship.previousSibling;
        }

        if (relationship.previousSibling != entt::null)
        {
            auto& previousSiblingRelationship =
                getComponent<EntityRelationship>(relationship.previousSibling);
            previousSiblingRelationship.nextSibling = relationship.nextSibling;
        }

        relationship.parent = parent;
        relationship.nextSibling = entt::null;
        relationship.previousSibling = entt::null;

        if (parent != entt::null)
        {
            auto& parentRelationship = getComponent<EntityRelationship>(parent);

            if (parentRelationship.firstChild != entt::null)
            {
                auto& firstChildRelationship =
                    getComponent<EntityRelationship>(parentRelationship.firstChild);

                firstChildRelationship.previousSibling = entity;
                relationship.nextSibling = parentRelationship.firstChild;
            }

            parentRelationship.firstChild = entity;
            parentRelationship.childCount++;
        }
    }
}  // namespace exage
