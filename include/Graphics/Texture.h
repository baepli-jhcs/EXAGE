#pragma once

#include "glm/glm.hpp"
#include "utils/classes.h"

namespace exage::Graphics
{
    struct EXAGE_EXPORT Sampler
    {
        Sampler() noexcept = default;
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

        [[nodiscard]] virtual auto getAnisotropy() const noexcept -> Anisotropy = 0;
        [[nodiscard]] virtual auto getFilter() const noexcept -> Filter = 0;
        [[nodiscard]] virtual auto getMipmapMode() const noexcept -> MipmapMode = 0;
        [[nodiscard]] virtual auto getLodBias() const noexcept -> float = 0;

        EXAGE_BASE_API(API, Sampler);
    };

    using TextureExtent = glm::uvec3;

    class EXAGE_EXPORT Texture
    {
    public:
        enum class Format : uint32_t;
        enum class Type : uint32_t;
        enum class Layout :uint32_t;
        BEGIN_RAW_BITFLAGS(Usage)
            RAW_FLAG(eTransferSource)
            RAW_FLAG(eTransferDestination)
            RAW_FLAG(eColorAttachment)
            RAW_FLAG(eDepthStencilAttachment)
        END_RAW_BITFLAGS(Usage)

        Texture() noexcept = default;
        virtual ~Texture() = default;
        EXAGE_DELETE_COPY(Texture);
        EXAGE_DEFAULT_MOVE(Texture);

        [[nodiscard]] virtual auto getExtent() const noexcept -> TextureExtent = 0;
        [[nodiscard]] virtual auto getFormat() const noexcept -> Format = 0;
        [[nodiscard]] virtual auto getType() const noexcept -> Type = 0;
        [[nodiscard]] virtual auto getLayout() const noexcept -> Layout = 0;
        [[nodiscard]] virtual auto getUsage() const noexcept -> Usage = 0;

        [[nodiscard]] virtual auto getLayerCount() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getMipLevelCount() const noexcept -> uint32_t = 0;

        [[nodiscard]] virtual auto getSampler() noexcept -> Sampler& = 0;
        [[nodiscard]] virtual auto getSampler() const noexcept -> const Sampler& = 0;


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
            eInputAttachment,
            eColorAttachment,
            eDepthStencilAttachment,
            eReadOnly,
            eTransferSource,
            eTransferDestination,
        };


        EXAGE_BASE_API(API, Texture);
    };

    struct SamplerCreateInfo
    {
        Sampler::Anisotropy anisotropy = Sampler::Anisotropy::eDisabled;
        Sampler::Filter filter = Sampler::Filter::eLinear;
        Sampler::MipmapMode mipmapMode = Sampler::MipmapMode::eLinear;
        float lodBias = 0.0f;
    };

    struct TextureCreateInfo
    {
        TextureExtent extent = glm::uvec3(1);
        Texture::Format format = Texture::Format::eRGBA8;
        Texture::Type type = Texture::Type::e2D;
        Texture::Usage usage = Texture::Usage::eColorAttachment;
        uint32_t arrayLayers = 1;
        uint32_t mipLevels = 1;

        SamplerCreateInfo samplerCreateInfo{};
    };
} // namespace exage::Graphics
