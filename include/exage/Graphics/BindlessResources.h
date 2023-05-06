#pragma once

#include <limits>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    struct ResourceID
    {
        uint32_t id = std::numeric_limits<uint32_t>::max();

        [[nodiscard]] auto valid() const noexcept -> bool
        {
            return id != std::numeric_limits<uint32_t>::max();
        }
    };

    struct BufferID : ResourceID
    {
    };

    struct TextureID : ResourceID
    {
    };

    class Buffer;
    class Texture;
}  // namespace exage::Graphics
