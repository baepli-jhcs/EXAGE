#include "exage/platform/Vulkan/VulkanTexture.h"

#include "exage/Core/Debug.h"

namespace exage::Graphics
{

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

        Texture::operator=(std::move(old));

        cleanup();

        _context = old._context;

        _allocation = old._allocation;
        _image = old._image;
        _imageView = old._imageView;

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
        viewInfo.viewType = toVulkanImageViewType(_type, _layerCount);
        viewInfo.image = _image;
        viewInfo.components = vk::ComponentMapping();
        viewInfo.format = format;
        viewInfo.subresourceRange =
            vk::ImageSubresourceRange(aspectFlags, 0, _mipLevelCount, 0, _layerCount);

        result = _context.get().getDevice().createImageView(&viewInfo, nullptr, &_imageView);
        checkVulkan(result);

        _id = _context.get().getResourceManager().bindTexture(*this);
    }
}  // namespace exage::Graphics
