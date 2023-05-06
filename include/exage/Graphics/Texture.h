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
        eRG8,
        eRG16,
        eRGB8,
        eRGB16,
        eRGBA8,
        eRGBA16,
        eR16f,
        eRG16f,
        eRGB16f,
        eRGBA16f,
        eR32f,
        eRG32f,
        eRGB32f,
        eRGBA32f,
        eDepth24Stencil8,
        eDepth32Stencil8,
        eBGRA8,
    };

    struct SamplerCreateInfo;

    struct EXAGE_EXPORT Sampler
    {
        virtual ~Sampler() = default;
        EXAGE_DELETE_COPY(Sampler);
        EXAGE_DEFAULT_MOVE(Sampler);

        enum class Anisotropy : uint32_t
        {
            eDisabled = 0,
            e1 = 1,
            e2 = 2,
            e4 = 4,
            e8 = 8,
            e16 = 16
        };

        enum class Filter : uint32_t
        {
            eNearest = 0,
            eLinear = 1
        };

        enum class MipmapMode : uint32_t
        {
            eNearest = 0,
            eLinear = 1
        };

        [[nodiscard]] auto getAnisotropy() const noexcept -> Anisotropy { return _anisotropy; }
        [[nodiscard]] auto getFilter() const noexcept -> Filter { return _filter; }
        [[nodiscard]] auto getMipmapMode() const noexcept -> MipmapMode { return _mipmapMode; }
        [[nodiscard]] auto getLodBias() const noexcept -> float { return _lodBias; }

        [[nodiscard]] auto getSamplerCreateInfo() const noexcept -> SamplerCreateInfo;

        EXAGE_BASE_API(API, Sampler);

      protected:
        Anisotropy _anisotropy;
        Filter _filter;
        MipmapMode _mipmapMode;
        float _lodBias;

        Sampler(Anisotropy anisotropy, Filter filter, MipmapMode mipmapMode, float lodBias) noexcept
            : _anisotropy(anisotropy)
            , _filter(filter)
            , _mipmapMode(mipmapMode)
            , _lodBias(lodBias)
        {
        }
    };

    using TextureExtent = glm::uvec3;

    class EXAGE_EXPORT Texture
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

        [[nodiscard]] virtual auto getSampler() noexcept -> Sampler& = 0;
        [[nodiscard]] virtual auto getSampler() const noexcept -> const Sampler& = 0;

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

    struct SamplerCreateInfo
    {
        Sampler::Anisotropy anisotropy = Sampler::Anisotropy::eDisabled;
        Sampler::Filter filter = Sampler::Filter::eLinear;
        Sampler::MipmapMode mipmapMode = Sampler::MipmapMode::eLinear;
        float lodBias = 0.0F;

        TextureID _id {};
    };

    struct TextureCreateInfo
    {
        TextureExtent extent = glm::uvec3(1);
        Format format = Format::eRGBA8;
        Texture::Type type = Texture::Type::e2D;
        Texture::Usage usage = Texture::UsageFlags::eSampled;
        uint32_t arrayLayers = 1;
        uint32_t mipLevels = 1;

        SamplerCreateInfo samplerCreateInfo {};
    };
}  // namespace exage::Graphics
