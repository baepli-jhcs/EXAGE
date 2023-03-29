#include "exage/platform/Vulkan/VulkanSwapchain.h"

#include "exage/platform/Vulkan/VulkanCommandBuffer.h"
#include "exage/platform/Vulkan/VulkanQueue.h"
#include "exage/platform/Vulkan/VulkanTexture.h"

namespace exage::Graphics
{
    VulkanSwapchain::VulkanSwapchain(VulkanContext& context,
                                     const SwapchainCreateInfo& createInfo) noexcept
        : _context(context)
        , _presentMode(createInfo.presentMode)
        , _extent(createInfo.window.getExtent())
    {
        createSurface(createInfo.window);
        createSwapchain();
    }

    constexpr auto DESIRED_FORMAT = toVulkanFormat(Texture::Format::eRGBA8);
    constexpr auto FALLBACK_FORMAT = toVulkanFormat(Texture::Format::eBGRA8);

    void VulkanSwapchain::createSurface(Window& window) noexcept
    {
        _surface = _context.get().createSurface(window);
    }

    void VulkanSwapchain::createSwapchain() noexcept
    {
        auto& context = _context.get();
        vkb::SwapchainBuilder builder {context.getPhysicalDevice(), context.getDevice(), _surface};
        builder.set_desired_present_mode(
            static_cast<VkPresentModeKHR>(toVulkanPresentMode(_presentMode)));

        vk::SurfaceFormatKHR surfaceFormat;
        surfaceFormat.format = DESIRED_FORMAT;
        surfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        builder.set_desired_format(surfaceFormat);

        surfaceFormat.format = FALLBACK_FORMAT;
        builder.add_fallback_format(surfaceFormat);
        builder.set_desired_extent(_extent.x, _extent.y);

        builder.set_old_swapchain(_oldSwapchain);

        builder.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                      | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        auto swapchainResult = builder.build();
        debugAssume(swapchainResult.has_value(), "Failed to create swapchain");
        _swapchain = swapchainResult.value();

        if (static_cast<vk::Format>(_swapchain.image_format) == DESIRED_FORMAT)
        {
            _format = Texture::Format::eRGBA8;
        }
        else if (static_cast<vk::Format>(_swapchain.image_format) == FALLBACK_FORMAT)
        {
            _format = Texture::Format::eBGRA8;
        }

        auto images = _swapchain.get_images();
        if (images)
        {
            _swapchainImages.resize(images.value().size());
            for (size_t i = 0; i < images.value().size(); ++i)
            {
                _swapchainImages[i] = images.value()[i];
            }

            _swapchainTransitioned = std::vector<bool>(_swapchainImages.size(), false);
        }
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        if (_swapchain != nullptr)
        {
            _context.get().waitIdle();
            vkb::destroy_swapchain(_swapchain);
        }

        if (_surface)
        {
            vkb::destroy_surface(_context.get().getInstance(), _surface);
        }
    }

    VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& old) noexcept
        : _context(old._context), _swapchain(old._swapchain), _surface(old._surface), _oldSwapchain(old._oldSwapchain), _swapchainImages(old._swapchainImages), _swapchainTransitioned(std::move(old._swapchainTransitioned)), _extent(old._extent), _format(old._format), _presentMode(old._presentMode)
    {
        
        
        
        
        
        
        
        

        old._surface = nullptr;
        old._swapchain = {};
        old._oldSwapchain = nullptr;
    }

    auto VulkanSwapchain::operator=(VulkanSwapchain&& old) noexcept -> VulkanSwapchain&
    {
        if (this == &old)
        {
            return *this;
        }

        if (_swapchain != nullptr)
        {
            _context.get().waitIdle();
            vkb::destroy_swapchain(_swapchain);
        }

        if (_surface)
        {
            vkb::destroy_surface(_context.get().getInstance(), _surface);
        }

        _context = old._context;

        _swapchain = old._swapchain;
        _surface = old._surface;
        _oldSwapchain = old._oldSwapchain;
        _swapchainImages = old._swapchainImages;
        _swapchainTransitioned = std::move(old._swapchainTransitioned);
        _extent = old._extent;
        _format = old._format;
        _presentMode = old._presentMode;

        old._surface = nullptr;
        old._swapchain = {};
        old._oldSwapchain = nullptr;
        return *this;
    }

    void VulkanSwapchain::resize(glm::uvec2 extent) noexcept
    {
        _extent = extent;
        const vkb::Swapchain swapchain = _swapchain;
        _oldSwapchain = swapchain.swapchain;
        createSwapchain();

        _context.get().waitIdle();
        vkb::destroy_swapchain(swapchain);
    }

    auto VulkanSwapchain::acquireNextImage() noexcept -> std::optional<Error>
    {
        const VulkanQueue& vulkanQueue = _context.get().getVulkanQueue();
        const vk::SwapchainKHR swapchain = _swapchain.swapchain;
        vk::ResultValue<uint32_t> const result = _context.get().getDevice().acquireNextImageKHR(
            swapchain,
            std::numeric_limits<uint64_t>::max(),
            vulkanQueue.getCurrentPresentSemaphore(),
            nullptr);
        if (result.result == vk::Result::eErrorOutOfDateKHR
            || result.result == vk::Result::eSuboptimalKHR)
        {
            return GraphicsError::eSwapchainOutOfDate;
        }
        checkVulkan(result.result);
        _currentImage = result.value;
        return std::nullopt;
    }

    void VulkanSwapchain::drawImage(CommandBuffer& commandBuffer,
                                    const std::shared_ptr<Texture>& texture) noexcept
    {
        const auto* vulkanTexture = texture->as<VulkanTexture>();
        debugAssume(vulkanTexture->getLayout() == Texture::Layout::eTransferSrc,
                    "Wrong texture layout");
        debugAssume(vulkanTexture->getType() == Texture::Type::e2D, "Wrong texture type");

        bool const transitioned = _swapchainTransitioned[_currentImage];

        std::function const commandFunction = [this, vulkanTexture, transitioned](CommandBuffer& cmd)
        {
            const auto* vulkanCommandBuffer = cmd.as<VulkanCommandBuffer>();
            const vk::CommandBuffer vkCommand = vulkanCommandBuffer->getCommandBuffer();

            glm::uvec3 const extent = vulkanTexture->getExtent();

            vk::ImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            vk::ImageMemoryBarrier barrier;
            barrier.oldLayout =
                transitioned ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eUndefined;
            barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = _swapchainImages[_currentImage];
            barrier.subresourceRange = subresourceRange;

            vkCommand.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                      vk::PipelineStageFlagBits::eTransfer,
                                      vk::DependencyFlags(),
                                      nullptr,
                                      nullptr,
                                      barrier);

            vk::ImageBlit imageBlit = {};
            imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcOffsets[1].x = static_cast<int32_t>(extent.x);
            imageBlit.srcOffsets[1].y = static_cast<int32_t>(extent.y);
            imageBlit.srcOffsets[1].z = 1;
            imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstOffsets[1].x = static_cast<int32_t>(_swapchain.extent.width);
            imageBlit.dstOffsets[1].y = static_cast<int32_t>(_swapchain.extent.height);
            imageBlit.dstOffsets[1].z = 1;

            vkCommand.blitImage(vulkanTexture->getImage(),
                                vk::ImageLayout::eTransferSrcOptimal,
                                _swapchainImages[_currentImage],
                                vk::ImageLayout::eTransferDstOptimal,
                                imageBlit,
                                vk::Filter::eLinear);

            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = _swapchainImages[_currentImage];
            barrier.subresourceRange = subresourceRange;

            vkCommand.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                      vk::PipelineStageFlagBits::eBottomOfPipe,
                                      vk::DependencyFlags(),
                                      nullptr,
                                      nullptr,
                                      barrier);
        };
        commandBuffer.userDefined(commandFunction);
        commandBuffer.insertDataDependency(texture);
    }

    auto VulkanSwapchain::getImage(uint32_t index) const noexcept -> vk::Image
    {
        return _swapchainImages[index];
    }
}  // namespace exage::Graphics
