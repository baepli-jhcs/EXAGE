﻿#pragma once
#include <entt/entity/registry.hpp>

#include "exage/Core/Core.h"
#include "exage/Scene/Entity.h"
#include "exage/Scene/Hierarchy.h"

namespace exage
{
    class EXAGE_EXPORT Scene
    {
      public:
        Scene() noexcept = default;
        ~Scene() = default;

        [[nodiscard]] auto createEntity(Entity parent = entt::null) noexcept -> Entity;
        void destroyEntity(Entity entity) noexcept;

        void updateHierarchy(bool calculateTransforms = true) noexcept;

        entt::registry& registry() noexcept { return _registry; }
        const entt::registry& registry() const noexcept { return _registry; }

        bool isValid(Entity entity) const noexcept { return _registry.valid(entity); }

        template<typename T, typename... Args>
        auto addComponent(Entity entity, Args&&... args) noexcept -> T&
        {
            return _registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename T>
        auto getComponent(Entity entity) noexcept -> T&
        {
			return _registry.get<T>(entity);
		}

        template<typename T>
        void removeComponent(Entity entity) noexcept
        {
			_registry.remove<T>(entity);
		}

        template<typename T>
        bool hasComponent(Entity entity) const noexcept
        {
            return _registry.all_of<T>(entity);
        }

        template<typename F>
        void forEachChild(Entity parent, F&& func) noexcept
        {
            auto& relationship = getComponent<EntityRelationship>(parent);
            auto child = relationship.firstChild;
            for (size_t i = 0; i < relationship.childCount; i++)
            {
                func(child);
                child = getComponent<EntityRelationship>(child).nextSibling;
            }
        }

      private:
        void calculateChildTransform(Transform3D& parentTransform, Entity entity) noexcept;

        entt::registry _registry;
    };
}  // namespace exage