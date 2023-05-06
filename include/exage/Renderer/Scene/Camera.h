#pragma once

#include <glm/glm.hpp>

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Utils/RAII.h"
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
    };

    struct CameraRenderInfo
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 viewProjection;
        alignas(16) glm::vec3 position;
    };

    EXAGE_EXPORT void setSceneCamera(Scene& scene, Entity cameraEntity) noexcept;
    [[nodiscard]] EXAGE_EXPORT auto getSceneCamera(Scene& scene) noexcept -> Entity;
}  // namespace exage::Renderer
