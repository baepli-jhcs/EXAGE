#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "exage/Graphics/Texture.h"

namespace exage::Renderer
{
    struct PointLight
    {
        glm::vec3 color;
        float intensity;
    };

    struct DirectionalLight
    {
        glm::vec3 color;
        float intensity;
    };

    struct SpotLight
    {
        glm::vec3 color;
        float intensity;
        float innerCutoff;
        float outerCutoff;
    };

    struct PointLightRenderInfo
    {
        struct Data
        {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 color;
            alignas(4) float intensity;
            alignas(4) uint32_t shadowMapIndex;
        };

        Data data;
        std::shared_ptr<Graphics::FrameBuffer> shadowMap;
    };

    struct DirectionalLightRenderInfo
    {
        struct Data
        {
            alignas(16) glm::vec3 direction;
            alignas(16) glm::vec3 color;
            alignas(4) float intensity;
            alignas(4) uint32_t shadowMapIndex;
        };

        Data data;
        std::shared_ptr<Graphics::FrameBuffer> shadowMap;
    };

    struct SpotLightRenderInfo
    {
        struct Data
        {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 direction;
            alignas(16) glm::vec3 color;
            alignas(4) float intensity;
            alignas(4) float innerCutoff;
            alignas(4) float outerCutoff;
            alignas(4) uint32_t shadowMapIndex;
        };

        Data data;
        std::shared_ptr<Graphics::FrameBuffer> shadowMap;
    };
}  // namespace exage::Renderer