#pragma once
#include <entt/entity/registry.hpp>

#include "Core/Core.h"
#include "Scene/Entity.h"
#include "Scene/Hierarchy.h"

namespace exage
{
    class EXAGE_EXPORT Scene
    {
      public:
        Scene() noexcept = default;
        ~Scene() = default;

        [[nodiscard]] auto createEntity(Entity parent = {}) noexcept -> Entity;
        void destroyEntity(Entity& entity) noexcept;

        entt::registry& registry() noexcept { return _registry; }
        const entt::registry& registry() const noexcept { return _registry; }

      private:
        entt::registry _registry;
    };

    template<typename T, typename... Args>
    auto Entity::addComponent(Args&&... args) const noexcept -> T&
    {
        return _scene->registry().emplace<T>(_handle, std::forward<Args>(args)...);
    }

    template<typename T>
    auto Entity::getComponent() const noexcept -> T&
    {
        return _scene->registry().get<T>(_handle);
    }

    template<typename T>
    void Entity::removeComponent() const noexcept
    {
        _scene->registry().remove<T>(_handle);
    }

    template<typename T>
    bool Entity::hasComponent() const noexcept
    {
        return _scene->registry().all_of<T>(_handle);
    }

    template<typename F>
    void Entity::forEachChild(F&& func) const noexcept
    {
        auto& relationship = getComponent<EntityRelationship>();
        auto child = relationship.firstChild;

        for (size_t i = 0; i < relationship.childCount; i++)
        {
            func(child);
            child = child.getComponent<EntityRelationship>().nextSibling;
        }
    }
}  // namespace exage
