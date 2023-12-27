#pragma once

#include "exage/Graphics/Utils/SlotBuffer.h"

namespace exage::Renderer
{
    struct GeometryData
    {
        Graphics::SlotBuffer vertexBuffer;
        Graphics::SlotBuffer indexBuffer;
    };

};  // namespace exage::Renderer