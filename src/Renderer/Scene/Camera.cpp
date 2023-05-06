#include "exage/Renderer/Scene/Camera.h"

namespace exage::Renderer
{
    struct ActiveCameraTag
    {
    };

    void setSceneCamera(Scene& scene, Entity cameraEntity) noexcept
    {
        auto view = scene.registry().view<ActiveCameraTag>();
        for (auto entity : view)
        {
            scene.registry().remove<ActiveCameraTag>(entity);
        }

        scene.registry().emplace<ActiveCameraTag>(cameraEntity);
    }

    auto getSceneCamera(Scene& scene) noexcept -> Entity
    {
        auto view = scene.registry().view<ActiveCameraTag>();
        for (auto entity : view)
        {
            return entity;
        }

        return entt::null;
    }
}  // namespace exage::Renderer
