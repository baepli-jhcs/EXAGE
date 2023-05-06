#pragma once

#include <glm/glm.hpp>

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Utils/RAII.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct TransformRenderInfo
    {
        glm::mat4 model;
        glm::mat4 normal;
        glm::mat4 modelViewProjection;
    };
}  // namespace exage::Renderer
