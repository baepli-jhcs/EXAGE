#include "exage/platform/Vulkan/VulkanTexture.h"

#include "exage/Core/Debug.h"

namespace exage::Graphics
{

    VulkanSampler::~VulkanSampler()
    {
        if (_sampler)
        {
            _context.get().getDevice().destroySampler(_sampler);
        }
    }

    VulkanSampler::VulkanSampler(VulkanSampler&& old) noexcept
        : Sampler(std::move(old))
        , _context(old._context)
        , _sampler(old._sampler)
    {
        old._sampler = nullptr;
    }

    auto VulkanSampler::operator=(VulkanSampler&& old) noexcept -> VulkanSampler&
    {
        if (this == &old)
        {
            return *this;
        }

        Sampler::operator=(std::move(old));

        if (_sampler)
        {
            _context.get().getDevice().destroySampler(_sampler);
        }

        _context = old._context;

        _sampler = old._sampler;
        old._sampler = nullptr;
        return *this;
    }

    VulkanSampler::VulkanSampler(VulkanContext& context,
                                 const SamplerCreateInfo& createInfo,
                                 uint32_t mipLevelCount) noexcept
        : Sampler(
            createInfo.anisotropy, createInfo.filter, createInfo.mipmapMode, createInfo.lodBias)
        , _context(context)
    {
        vk::Filter const filter = toVulkanFilter(_filter);
        vk::SamplerMipmapMode const mipmapMode = toVulkanSamplerMipmapMode(_mipmapMode);

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable =
            static_cast<vk::Bool32>(_anisotropy != Anisotropy::eDisabled);

        switch (_anisotropy)
        {
            case Anisotropy::eDisabled:
            case Anisotropy::e1:
                samplerInfo.maxAnisotropy = 1.0F;
                break;
            case Anisotropy::e2:
                samplerInfo.maxAnisotropy = 2.0F;
                break;
            case Anisotropy::e4:
                samplerInfo.maxAnisotropy = 4.0F;
                break;
            case Anisotropy::e8:
                samplerInfo.maxAnisotropy = 8.0F;
                break;
            case Anisotropy::e16:
                samplerInfo.maxAnisotropy = 16.0F;
                break;
        }
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerInfo.unnormalizedCoordinates = 0U;
        samplerInfo.compareEnable = 0U;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.mipLodBias = _lodBias;
        samplerInfo.minLod = 0.0F;
        samplerInfo.maxLod = static_cast<float>(mipLevelCount);

        const vk::Result result =
            _context.get().getDevice().createSampler(&samplerInfo, nullptr, &_sampler);
        checkVulkan(result);
    }

    VulkanTexture::~VulkanTexture()
    {
        cleanup();
    }

    void VulkanTexture::cleanup() noexcept
    {
        if (_id.valid())
        {
            _context.get().getResourceManager().unbindTexture(_id);
        }

        if (_imageView)
        {
            _context.get().getDevice().destroyImageView(_imageView);
        }

        if (_image)
        {
            _context.get().getDevice().destroyImage(_image);
        }

        if (_allocation)
        {
            _context.get().getAllocator().freeMemory(_allocation);
        }
    }

    VulkanTexture::VulkanTexture(VulkanTexture&& old) noexcept
        : Texture(std::move(old))
        , _context(old._context)
        , _allocation(old._allocation)
        , _image(old._image)
        , _imageView(old._imageView)
        , _sampler(std::move(old._sampler))
    {
        old._id = {};

        old._allocation = nullptr;
        old._image = nullptr;
        old._imageView = nullptr;
    }

    auto VulkanTexture::operator=(VulkanTexture&& old) noexcept -> VulkanTexture&
    {
        if (this == &old)
        {
            return *this;
        }

        cleanup();

        _context = old._context;

        _allocation = old._allocation;
        _image = old._image;
        _imageView = old._imageView;
        _sampler = std::move(old._sampler);

        old._id = {};

        old._allocation = nullptr;
        old._image = nullptr;
        old._imageView = nullptr;

        return *this;
    }

    VulkanTexture::VulkanTexture(VulkanContext& context,
                                 const TextureCreateInfo& createInfo) noexcept
        : Texture(createInfo.extent,
                  createInfo.format,
                  createInfo.type,
                  createInfo.usage,
                  createInfo.arrayLayers,
                  createInfo.mipLevels)
        , _context(context)
    {
        debugAssume(_extent.x > 0 && _extent.y > 0 && _extent.z > 0, "Invalid texture extent");

        vk::ImageUsageFlags const usage = toVulkanImageUsageFlags(_usage);
        vk::ImageAspectFlags const aspectFlags = toVulkanImageAspectFlags(_usage);
        vk::Format const format = toVulkanFormat(_format);

        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = toVulkanImageType(_type);
        imageInfo.extent = vk::Extent3D {_extent.x, _extent.y, _extent.z};
        imageInfo.mipLevels = _mipLevelCount;
        imageInfo.arrayLayers = _layerCount;
        imageInfo.format = format;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = usage;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::ImageCreateFlags const flags = _type == Type::eCube
            ? vk::ImageCreateFlagBits::eCubeCompatible
            : vk::ImageCreateFlags();
        imageInfo.flags = flags;

        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = vma::MemoryUsage::eAuto;

        vk::Result result = _context.get().getAllocator().createImage(
            &imageInfo, &allocInfo, &_image, &_allocation, nullptr);
        checkVulkan(result);

        vk::ImageViewCreateInfo viewInfo;
        viewInfo.viewType = toVulkanImageViewType(_type);
        viewInfo.image = _image;
        viewInfo.components = vk::ComponentMapping();
        viewInfo.format = format;
        viewInfo.subresourceRange =
            vk::ImageSubresourceRange(aspectFlags, 0, _mipLevelCount, 0, _layerCount);

        result = _context.get().getDevice().createImageView(&viewInfo, nullptr, &_imageView);
        checkVulkan(result);

        _sampler = VulkanSampler {_context, createInfo.samplerCreateInfo, _mipLevelCount};

        _id = _context.get().getResourceManager().bindTexture(*this);
    }
}  // namespace exage::Graphics
