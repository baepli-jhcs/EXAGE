#pragma once

#include <entt/entity/registry.hpp>

#include "Core/Core.h"

namespace exage
{
    class Scene;

    class EXAGE_EXPORT Entity
    {
      public:
        Entity() noexcept = default;
        Entity(entt::entity entityHandle, Scene& scene) noexcept;

        entt::entity getHandle() const noexcept { return _handle; }

        bool isValid() const noexcept;

        template<typename T, typename... Args>
        auto addComponent(Args&&... args) const noexcept -> T&;

        template<typename T>
        [[nodiscard]] auto getComponent() const noexcept -> T&;

        template<typename T>
        void removeComponent() const noexcept;

        template<typename T>
        bool hasComponent() const noexcept;

        template<typename F>
        void forEachChild(F&& func) const noexcept;

        bool operator==(const Entity& other) const noexcept
        {
            return _handle == other._handle && _scene == other._scene;
        }

      private:
        entt::entity _handle = entt::null;
        Scene* _scene = nullptr;
    };
}  // namespace exage
