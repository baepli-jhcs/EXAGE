#pragma once

#include <limits>

#include "exage/Core/Core.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Texture.h"

namespace exage::Graphics
{
    struct ResourceID
    {
        uint32_t id;

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

    class EXAGE_EXPORT ResourceManager
    {
      public:
        ResourceManager() noexcept = default;
        virtual ~ResourceManager() = default;

        EXAGE_DEFAULT_COPY(ResourceManager);
        EXAGE_DEFAULT_MOVE(ResourceManager);

        [[nodiscard]] virtual auto bindBuffer(Buffer& buffer) noexcept -> BufferID = 0;
        [[nodiscard]] virtual auto bindTexture(Texture& texture) noexcept -> TextureID = 0;

        virtual void unbindBuffer(BufferID buffer) noexcept = 0;
        virtual void unbindTexture(TextureID texture) noexcept = 0;

        EXAGE_BASE_API(API, ResourceManager);
    };
}  // namespace exage::Graphics
