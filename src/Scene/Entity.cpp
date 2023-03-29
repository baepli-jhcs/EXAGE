#include "exage/Scene/Entity.h"

#include "exage/Scene/Scene.h"

namespace exage
{
    Entity::Entity(const entt::entity entityHandle, Scene& scene) noexcept
        : _handle(entityHandle)
        , _scene(&scene)
    {
    }

    auto Entity::isValid() const noexcept -> bool
    {
        if (_handle == entt::null) {
            return false;
}
        return _scene->registry().valid(_handle);
    }
}  // namespace exage
