#include "Scene/Scene.h"

#include "Scene/Hierarchy.h"
#include "Scene/Entity.h"

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
}  // namespace exage
