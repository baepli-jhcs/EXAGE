#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "exage/Graphics/Texture.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Locations.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    constexpr auto DEFAULT_SHADOW_BIAS = 0.05F;

    struct PointLight
    {
        glm::vec3 color;
        float intensity;
        float physicalRadius;
        float attenuationRadius;
        bool castShadow;
        float shadowBias = DEFAULT_SHADOW_BIAS;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(color, intensity, physicalRadius, attenuationRadius, castShadow, shadowBias);
        }
    };

    struct DirectionalLight
    {
        glm::vec3 color;
        float intensity;
        bool castShadow;
        float shadowBias = DEFAULT_SHADOW_BIAS;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(color, intensity, castShadow, shadowBias);
        }
    };

    struct SpotLight
    {
        glm::vec3 color;
        float intensity;
        float innerCutoff;
        float outerCutoff;
        float physicalRadius;
        float attenuationRadius;
        bool castShadow;
        float shadowBias = DEFAULT_SHADOW_BIAS;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(color,
                    intensity,
                    innerCutoff,
                    outerCutoff,
                    physicalRadius,
                    attenuationRadius,
                    castShadow,
                    shadowBias);
        }
    };

    struct PointLightRenderInfo
    {
        uint32_t arrayIndex;
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        float physicalRadius;
        float attenuationRadius;
        std::shared_ptr<Graphics::FrameBuffer> shadowMap;
        int32_t shadowMapIndex = -1;
        float shadowBias = DEFAULT_SHADOW_BIAS;
    };

    struct PointLightRenderArray
    {
        struct Data
        {
            struct ArrayItem
            {
                alignas(16) glm::vec3 position;
                alignas(16) glm::vec3 color;
                float intensity;
                float physicalRadius;
                float attenuationRadius;
                float shadowBias = DEFAULT_SHADOW_BIAS;
                int32_t shadowMapIndex = -1;
                // Padding to 64 bytes
            };

            constexpr static auto POINT_LIGHT_ARRAY_ITEM_SIZE = sizeof(ArrayItem);

            uint32_t count;
        };

        std::optional<Graphics::ResizableDynamicBuffer> buffer;
    };

    constexpr auto MAX_CASCADE_LEVELS = 5;

    struct DirectionalLightRenderInfo
    {
        uint32_t arrayIndex;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        std::shared_ptr<Graphics::FrameBuffer> shadowMap;
        int32_t shadowMapIndex = -1;
        float shadowBias = DEFAULT_SHADOW_BIAS;
        uint8_t cascadeLevels;
        glm::mat4 cascadeViewProjections[MAX_CASCADE_LEVELS];
        float cascadeSplits[MAX_CASCADE_LEVELS];
    };

    struct DirectionalLightRenderArray
    {
        struct Data
        {
            struct ArrayItem
            {
                glm::mat4 cascadeViewProjections[MAX_CASCADE_LEVELS];
                alignas(16) glm::vec3 direction;
                float intensity;
                alignas(16) glm::vec3 color;
                int32_t shadowMapIndex = -1;
                float shadowBias = DEFAULT_SHADOW_BIAS;
                uint32_t cascadeLevels;
                float cascadeSplits[MAX_CASCADE_LEVELS];
                // Padding to 400 bytes
                float padding;
            };

            constexpr static auto DIRECTIONAL_LIGHT_ARRAY_ITEM_SIZE = sizeof(ArrayItem);

            uint32_t count;
        };

        std::optional<Graphics::ResizableDynamicBuffer> buffer;
    };

    struct SpotLightRenderInfo
    {
        uint32_t arrayIndex;
        std::shared_ptr<Graphics::FrameBuffer> shadowMap;
        int32_t shadowMapIndex = -1;
    };

    struct SpotLightRenderArray
    {
        struct Data
        {
            struct ArrayItem
            {
                glm::vec3 position;
                glm::vec3 direction;
                glm::vec3 color;
                float intensity;
                float innerCutoff;
                float outerCutoff;
                float physicalRadius;
                float attenuationRadius;
                int32_t shadowMapIndex = -1;
                float shadowBias = DEFAULT_SHADOW_BIAS;
                // Padding to 72 bytes
                float padding[2];
            };

            constexpr static auto SPOT_LIGHT_ARRAY_ITEM_SIZE = sizeof(ArrayItem);

            uint32_t count;
        };

        std::optional<Graphics::ResizableDynamicBuffer> buffer;
    };
}  // namespace exage::Renderer