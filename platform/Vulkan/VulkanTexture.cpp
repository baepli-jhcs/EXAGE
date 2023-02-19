#include "VulkanTexture.h"

namespace exage::Graphics
{
    auto VulkanSampler::create(VulkanContext& context,
                               SamplerCreateInfo& createInfo,
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
                                 SamplerCreateInfo& createInfo) noexcept
        : _context(context)
          , _anisotropy(createInfo.anisotropy)
          , _filter(createInfo.filter)
          , _mipmapMode(createInfo.mipmapMode)
          , _lodBias(createInfo.lodBias) { }

    auto VulkanSampler::init(uint32_t mipLevelCount) noexcept -> std::optional<Error>
    {
        vk::Filter filter = toVulkanFilter(_filter);
        vk::SamplerMipmapMode mipmapMode = toVulkanSamplerMipmapMode(_mipmapMode);

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
                               TextureCreateInfo& createInfo) noexcept -> tl::expected<
        std::unique_ptr<Texture>, Error>
    {
        std::unique_ptr<VulkanTexture> texture{new VulkanTexture{context, createInfo}};
        if (std::optional<Error> error = texture->init(createInfo.samplerCreateInfo); error.
            has_value())
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

    VulkanTexture::VulkanTexture(VulkanContext& context, TextureCreateInfo& createInfo) noexcept
        : _context(context)
          , _extent(createInfo.extent)
          , _format(createInfo.format)
          , _type(createInfo.type), _usage(createInfo.usage)
          , _layerCount(createInfo.arrayLayers), _mipLevelCount(createInfo.mipLevels) { }


    auto VulkanTexture::init(SamplerCreateInfo& samplerInfo) noexcept -> std::optional<Error>
    {
        vk::ImageUsageFlags usage = toVulkanImageUsageFlags(_usage);
        vk::ImageAspectFlags aspectFlags = toVulkanImageAspectFlags(_usage);
        vk::Format format = toVulkanFormat(_format);

        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = toVulkanImageType(_type);
        imageInfo.extent = vk::Extent3D{_extent.x, _extent.y, _extent.z};
        imageInfo.mipLevels = _mipLevelCount;
        imageInfo.arrayLayers = _layerCount;
        imageInfo.format = format;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = usage;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::ImageCreateFlags flags = _type == Type::eCube
            ? vk::ImageCreateFlagBits::eCubeCompatible
            : vk::ImageCreateFlags();
        imageInfo.flags = flags;

        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = vma::MemoryUsage::eGpuOnly;

        vk::Result result = _context.get().getAllocator().createImage(
            &imageInfo,
            &allocInfo,
            &_image,
            &_allocation,
            nullptr);

        if (result != vk::Result::eSuccess)
            return ErrorCode::eTextureCreationFailed;

        vk::ImageViewCreateInfo viewInfo;
        viewInfo.viewType = toVulkanImageViewType(_type);
        viewInfo.image = _image;
        viewInfo.components = vk::ComponentMapping();
        viewInfo.format = format;

        result = _context.get().getDevice().createImageView(&viewInfo, nullptr, &_imageView);
        if (result != vk::Result::eSuccess)
            return ErrorCode::eTextureViewCreationFailed;

        tl::expected sampler = VulkanSampler::create(_context, samplerInfo, _mipLevelCount);
        if (!sampler.has_value())
        {
            return sampler.error();
        }

        return std::nullopt;
    }
} // namespace exage::Graphics
