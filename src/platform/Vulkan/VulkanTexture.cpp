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
        auto idDestruction = [this](auto& id)
        {
            if (id.valid())
            {
                _context.get().getResourceManager().unbindTexture(id);
            }
        };

        idDestruction(_colorBindlessID);
        idDestruction(_depthBindlessID);
        idDestruction(_stencilBindlessID);

        if (_firstImageView)
        {
            _context.get().getDevice().destroyImageView(_firstImageView);
        }

        if (_secondImageView)
        {
            _context.get().getDevice().destroyImageView(_secondImageView);
        }

        if (_depthStencilImageView)
        {
            _context.get().getDevice().destroyImageView(_depthStencilImageView);
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
        , _firstImageView(old._firstImageView)
        , _secondImageView(old._secondImageView)
        , _depthStencilImageView(old._depthStencilImageView)
    {
        old._colorBindlessID = {};
        old._depthBindlessID = {};
        old._stencilBindlessID = {};

        old._allocation = nullptr;
        old._image = nullptr;
        old._firstImageView = nullptr;
        old._secondImageView = nullptr;
        old._depthStencilImageView = nullptr;
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
        _firstImageView = old._firstImageView;
        _secondImageView = old._secondImageView;
        _depthStencilImageView = old._depthStencilImageView;

        old._colorBindlessID = {};
        old._depthBindlessID = {};
        old._stencilBindlessID = {};

        old._allocation = nullptr;
        old._image = nullptr;
        old._firstImageView = nullptr;
        old._secondImageView = nullptr;
        old._depthStencilImageView = nullptr;

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

        if (aspectFlags & vk::ImageAspectFlagBits::eColor)
        {
            result =
                _context.get().getDevice().createImageView(&viewInfo, nullptr, &_firstImageView);
            checkVulkan(result);
        }
        else
        {
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            result =
                _context.get().getDevice().createImageView(&viewInfo, nullptr, &_firstImageView);
            checkVulkan(result);

            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eStencil;
            result =
                _context.get().getDevice().createImageView(&viewInfo, nullptr, &_secondImageView);
            checkVulkan(result);

            viewInfo.subresourceRange.aspectMask = aspectFlags;
            result = _context.get().getDevice().createImageView(
                &viewInfo, nullptr, &_depthStencilImageView);
            checkVulkan(result);
        }

        if (_format == Format::eDepth24Stencil8 || _format == Format::eDepth32Stencil8)
        {
            auto ids = _context.get().getResourceManager().bindDepthStencilTexture(*this);
            _depthBindlessID = ids.first;
            _stencilBindlessID = ids.second;
        }
        else
        {
            _colorBindlessID = _context.get().getResourceManager().bindTexture(*this);
        }
    }

    auto VulkanTexture::getImageView(Aspect aspect) const noexcept -> vk::ImageView
    {
        switch (aspect)
        {
            case Aspect::eColor:
                return _firstImageView;
            case Aspect::eDepth:
                return _firstImageView;
            case Aspect::eStencil:
                return _secondImageView;
        }

        return {};
    }

}  // namespace exage::Graphics
