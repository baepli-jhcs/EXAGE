#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "exage/Graphics/Texture.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct PointLight
    {
        glm::vec3 color;
        float intensity;
        float physicalRadius;
        float attenuationRadius;
        bool castShadow;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(color, intensity, physicalRadius, attenuationRadius, castShadow);
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

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(color,
                    intensity,
                    innerCutoff,
                    outerCutoff,
                    physicalRadius,
                    attenuationRadius,
                    castShadow);
        }
    };

    struct DirectionalLight
    {
        glm::vec3 color;
        float intensity;
        bool castShadow;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(color, intensity, castShadow);
        }
    };
}  // namespace exage::Renderer