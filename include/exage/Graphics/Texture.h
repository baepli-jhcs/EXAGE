#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/BindlessResources.h"
#include "exage/Graphics/Context.h"
#include "exage/utils/classes.h"
#include "glm/glm.hpp"

namespace exage::Graphics
{
    enum class Format : uint32_t
    {
        eR8,
        eR16,
        eR16f,
        eR32f,
        eRG8,
        eRG16,
        eRG16f,
        eRG32f,
        eRGBA8,
        eRGBA16,
        eRGBA16f,
        eRGBA32f,
        eDepth24Stencil8,
        eDepth32Stencil8,
        eBGRA8,

        // Compressed Formats
        eBC1RGBA8,
        eBC3RGBA8,
        eBC4R8,
        eBC5RG8,
        eBC7RGBA8,
        eASTC4x4RGBA8,
        eASTC6x6RGBA8,
        eETC2RGBA8,
    };

    using TextureExtent = glm::uvec3;

    class Texture
    {
      public:
        enum class Type : uint32_t;
        enum class Layout : uint32_t;

        enum class UsageFlags : uint32_t
        {
            eTransferSrc = 1 << 0,
            eTransferDst = 1 << 1,
            eSampled = 1 << 2,
            eStorage = 1 << 3,
            eColorAttachment = 1 << 4,
            eDepthStencilAttachment = 1 << 5,
        };

        using Usage = Flags<UsageFlags>;

        virtual ~Texture() = default;
        EXAGE_DELETE_COPY(Texture);
        EXAGE_DEFAULT_MOVE(Texture);

        [[nodiscard]] auto getExtent() const noexcept -> TextureExtent { return _extent; }
        [[nodiscard]] auto getFormat() const noexcept -> Format { return _format; }
        [[nodiscard]] auto getType() const noexcept -> Type { return _type; }
        [[nodiscard]] auto getLayout() const noexcept -> Layout { return _layout; }
        [[nodiscard]] auto getUsage() const noexcept -> Usage { return _usage; }

        [[nodiscard]] auto getLayerCount() const noexcept -> uint32_t { return _layerCount; }
        [[nodiscard]] auto getMipLevelCount() const noexcept -> uint32_t { return _mipLevelCount; }

        [[nodiscard]] auto getBindlessID() const noexcept -> TextureID { return _id; }

        [[nodiscard]] auto getTextureCreateInfo() const noexcept -> TextureCreateInfo;

        enum class Type : uint32_t
        {
            e1D,
            e2D,
            e3D,
            eCube,
        };

        enum class Layout : uint32_t
        {
            eUndefined,
            eColorAttachment,
            eDepthStencilAttachment,
            eShaderReadOnly,
            eTransferSrc,
            eTransferDst,
            ePresent,
            eStorage,
            eDepthStencilReadOnly,
        };

        EXAGE_BASE_API(API, Texture);

      protected:
        TextureExtent _extent;
        Format _format;
        Type _type;
        Layout _layout = Layout::eUndefined;
        Usage _usage;
        uint32_t _layerCount;
        uint32_t _mipLevelCount;

        TextureID _id {};

        Texture(TextureExtent extent,
                Format format,
                Type type,
                Usage usage,
                uint32_t layerCount,
                uint32_t mipLevelCount) noexcept
            : _extent(extent)
            , _format(format)
            , _type(type)
            , _usage(usage)
            , _layerCount(layerCount)
            , _mipLevelCount(mipLevelCount)
        {
        }

        friend class VulkanCommandBuffer;
    };
    EXAGE_ENABLE_FLAGS(Texture::Usage)

    struct TextureCreateInfo
    {
        TextureExtent extent = glm::uvec3(1);
        Format format = Format::eRGBA8;
        Texture::Type type = Texture::Type::e2D;
        Texture::Usage usage = Texture::UsageFlags::eSampled;
        uint32_t arrayLayers = 1;
        uint32_t mipLevels = 1;
    };
}  // namespace exage::Graphics
