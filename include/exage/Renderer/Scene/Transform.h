#pragma once

#include <glm/glm.hpp>

#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct TransformRenderInfo
    {
        struct Data
        {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 normal;
            alignas(16) glm::mat4 modelViewProjection;
        };

        Data data;
        std::optional<Graphics::DynamicFixedBuffer> buffer;
    };
}  // namespace exage::Renderer
