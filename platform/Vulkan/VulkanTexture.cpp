#include "VulkanTexture.h"

namespace exage::Graphics
{
    auto VulkanSampler::create(VulkanContext& context,
                               const SamplerCreateInfo createInfo,
                               uint32_t mipLevelCount) noexcept -> tl::expected<
        VulkanSampler, Error>
    {
        VulkanSampler sampler{context, createInfo};
        if (std::optional<Error> error = sampler.init(mipLevelCount); error.has_value())
        {
            return tl::make_unexpected(error.value());
        }
        return sampler;
    }

    VulkanSampler::~VulkanSampler()
    {
        if (_sampler)
        {
            _context.get().getDevice().destroySampler(_sampler);
        }
    }

    VulkanSampler::VulkanSampler(VulkanContext& context,
                                 const SamplerCreateInfo createInfo) noexcept
        : _context(context)
          , _anisotropy(createInfo.anisotropy)
          , _filter(createInfo.filter)
          , _mipmapMode(createInfo.mipmapMode)
          , _lodBias(createInfo.lodBias) { }

    auto VulkanSampler::init(uint32_t mipLevelCount) noexcept -> std::optional<Error>
    {
        vk::Filter filter;
        switch (_filter)
        {
            case Filter::eNearest:
                filter = vk::Filter::eNearest;
                break;
            case Filter::eLinear:
                filter = vk::Filter::eLinear;
                break;
            default:
                return ErrorCode::eInvalidEnum;
        }

        vk::SamplerMipmapMode mipmapMode;
        switch (_mipmapMode)
        {
            case MipmapMode::eNearest:
                mipmapMode = vk::SamplerMipmapMode::eNearest;
                break;
            case MipmapMode::eLinear:
                mipmapMode = vk::SamplerMipmapMode::eLinear;
                break;
            default:
                return ErrorCode::eInvalidEnum;
        }

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = _anisotropy != Anisotropy::eDisabled;
        samplerInfo.maxAnisotropy = static_cast<float>(_anisotropy);
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerInfo.unnormalizedCoordinates = false;
        samplerInfo.compareEnable = false;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.mipLodBias = _lodBias;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevelCount);

        const vk::Result result =
            _context.get().getDevice().createSampler(&samplerInfo, nullptr, &_sampler);

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eSamplerCreationFailed;
        }

        return std::nullopt;
    }

    auto VulkanTexture::create(VulkanContext& context,
                               TextureCreateInfo createInfo) noexcept -> tl::expected<
        std::unique_ptr<Texture>, Error>
    {
        std::unique_ptr<VulkanTexture> texture =
            std::make_unique<VulkanTexture>(context, createInfo);
        if (std::optional<Error> error = texture->init(); error.has_value())
        {
            return tl::make_unexpected(error.value());
        }
        return texture;
    }

    VulkanTexture::~VulkanTexture()
    {
        _context.get().getDevice().destroyImageView(_imageView);
        _context.get().getDevice().destroyImage(_image);
        _context.get().getAllocator().freeMemory(_allocation);
    }

    VulkanTexture::VulkanTexture(VulkanContext& context, TextureCreateInfo createInfo) noexcept
        : _context(context)
          , _extent(createInfo.extent)
          , _format(createInfo.format)
          , _type(createInfo.type), _usage(createInfo.usage) { }


    auto VulkanTexture::init() noexcept -> std::optional<Error>
    {
        
    }
} // namespace exage::Graphics
