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

        enum class Anisotropy
        {
            eDisabled,
            e1 = 1,
            e2 = 2,
            e4 = 4,
            e8 = 8,
            e16 = 16,
        };

        enum class Filter
        {
            eNearest,
            eLinear,
        };

        enum class MipmapMode
        {
            eNearest,
            eLinear,
        };

        [[nodiscard]] virtual auto getAnisotropy() const noexcept -> Anisotropy = 0;
        [[nodiscard]] virtual auto getFilter() const noexcept -> Filter = 0;
        [[nodiscard]] virtual auto getMipmapMode() const noexcept -> MipmapMode = 0;
        [[nodiscard]] virtual auto getLodBias() const noexcept -> float = 0;

        EXAGE_BASE_API(API, Sampler);
    };

    using TextureExtent = std::variant<glm::uvec1, glm::uvec2, glm::uvec3>;

    class EXAGE_EXPORT Texture
    {
    public:
        enum class Format;
        enum class Type;
        enum class Layout;
        enum class Usage : uint32_t;

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

        enum class Format
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
            // ONLY USE FOR SWAPCHAIN
        };

        enum class Type
        {
            e1D,
            e2D,
            e3D,
        };

        enum class Layout
        {
            eUndefined,
            eDepthStencilAttachment,
            eReadOnly,
            eAttachment,
            eTransferSource,
            eTransferDestination,
        };

        enum class Usage : uint32_t
        {
            eTransferSource = 0 << 0,
            eTransferDestination = 1 << 0,
            eStorage = 1 << 1,
            eColorAttachment = 1 << 2,
            eDepthStencilAttachment = 1 << 3,
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
        size_t mipLevels = 1;
        size_t arrayLayers = 1;

        SamplerCreateInfo samplerCreateInfo{};
    };
} // namespace exage::Graphics
