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
            glm::mat4 model;
            glm::mat4 normal;
            glm::mat4 modelViewProjection;
        };

        Data data;
        std::optional<Graphics::DynamicFixedBuffer> buffer;
    };
}  // namespace exage::Renderer
