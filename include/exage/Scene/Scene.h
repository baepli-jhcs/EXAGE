#pragma once
#include <memory_resource>

#include <entt/entity/registry.hpp>

#include "exage/Core/Core.h"
#include "exage/Scene/Entity.h"
#include "exage/Scene/Hierarchy.h"

namespace exage
{
    class Scene
    {
      public:
        using Registry = entt::registry;  // May swap for an allocator in the future

        Scene() = default;
        ~Scene() = default;

        EXAGE_DELETE_COPY(Scene);
        EXAGE_DEFAULT_MOVE(Scene);

        auto createEntity(Entity parent = entt::null) noexcept -> Entity;
        void destroyEntity(Entity entity) noexcept;

        void updateHierarchy(bool calculateTransforms = true) noexcept;

        [[nodiscard]] auto registry() noexcept -> Registry& { return _registry; }
        [[nodiscard]] auto registry() const noexcept -> const Registry& { return _registry; }

        [[nodiscard]] auto isValid(Entity entity) const noexcept -> bool
        {
            return _registry.valid(entity);
        }

        template<typename T, typename... Args>
        auto addComponent(Entity entity, Args&&... args) noexcept -> T&
        {
            return _registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T>
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
        [[nodiscard]] auto hasComponent(Entity entity) const noexcept -> bool
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

        template<typename F>
        void forEachRoot(F&& func) noexcept
        {
            auto view = _registry.view<RootEntity>();
            for (auto entity : view)
            {
                func(entity);
            }
        }

        void setParent(Entity entity, Entity parent) noexcept;

      private:
        void calculateChildTransform(Transform3D& parentTransform, Entity entity) noexcept;

        Registry _registry;
    };
}  // namespace exage
