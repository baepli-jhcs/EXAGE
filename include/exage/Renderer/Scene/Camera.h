#pragma once

#include <glm/glm.hpp>

#include "entt/entity/fwd.hpp"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Locations.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct Camera
    {
        float fov = glm::radians(45.0f);
        float near = 0.1f;
        float far = 1000.0f;

        float exposure = 1.0f;
        float gamma = 2.2f;

        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(fov, near, far, exposure, gamma);
        }
    };

    struct CameraRenderInfo
    {
        struct Data
        {
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 projection;
            alignas(16) glm::mat4 viewProjection;
            alignas(16) glm::vec3 position;
        };

        Data data;
        std::optional<Graphics::DynamicFixedBuffer> buffer;
    };

    void setSceneCamera(Scene& scene, Entity cameraEntity) noexcept;
    [[nodiscard]] auto getSceneCamera(Scene& scene) noexcept -> Entity;

    inline auto getCameraRenderInfo(Scene& scene) noexcept -> CameraRenderInfo&
    {
        return scene.registry()
            .storage<CameraRenderInfo>(CURRENT_CAMERA_RENDER_INFO)
            .get(getSceneCamera(scene));
    }

    inline auto getCameraComponent(Scene& scene) noexcept -> Camera&
    {
        return scene.registry().storage<Camera>(CURRENT_CAMERA).get(getSceneCamera(scene));
    }

}  // namespace exage::Renderer
