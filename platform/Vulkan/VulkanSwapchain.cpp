#include "VulkanSwapchain.h"

#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanTexture.h"

namespace exage::Graphics
{
    VulkanSwapchain::VulkanSwapchain(VulkanContext& context,
                                     const SwapchainCreateInfo& createInfo) noexcept
        : _context(context)
          , _presentMode(createInfo.presentMode), _extent(createInfo.window.getExtent()) { }


    constexpr auto DESIRED_FORMAT = toVulkanFormat(Texture::Format::eRGBA8);
    constexpr auto FALLBACK_FORMAT = toVulkanFormat(Texture::Format::eBGRA8);

    auto VulkanSwapchain::init(Window& window) noexcept -> std::optional<Error>
    {
        auto result = createSurface(window);
        if (result.has_value())
        {
            return result;
        }
        result = createSwapchain();
        if (result.has_value())
        {
            return result;
        }
        return std::nullopt;
    }

    std::optional<Error> VulkanSwapchain::createSurface(Window& window) noexcept
    {
        auto& context = _context.get();

        tl::expected<vk::SurfaceKHR, Error> surfaceResult = context.createSurface(window);
        if (!surfaceResult)
        {
            return surfaceResult.error();
        }
        _surface = surfaceResult.value();
        return std::nullopt;
    }

    std::optional<Error> VulkanSwapchain::createSwapchain() noexcept
    {
        auto& context = _context.get();
        vkb::SwapchainBuilder builder{context.getPhysicalDevice(), context.getDevice(), _surface};
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
        if (!swapchainResult)
        {
            return ErrorCode::eSwapchainCreationFailed;
        }
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
        }

        return std::nullopt;
    }


    auto VulkanSwapchain::create(VulkanContext& context,
                                 const SwapchainCreateInfo& createInfo) noexcept
    -> tl::expected<VulkanSwapchain, Error>
    {
        VulkanSwapchain swapchain(context, createInfo);
        std::optional<Error> result = swapchain.init(createInfo.window);
        if (result.has_value())
        {
            return tl::make_unexpected(result.value());
        }

        return swapchain;
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        _context.get().waitIdle();

        if (_swapchain)
        {
            vkb::destroy_swapchain(_swapchain);
        }

        if (_surface)
        {
            vkb::destroy_surface(_context.get().getInstance(), _surface);
        }
    }

    VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& old) noexcept
        : _context(old._context)
    {
        *this = std::move(old);
    }

    auto VulkanSwapchain::operator=(VulkanSwapchain&& old) noexcept -> VulkanSwapchain&
    {
        _swapchain = old._swapchain;
        _surface = old._surface;
        _oldSwapchain = old._oldSwapchain;
        _swapchainImages = old._swapchainImages;
        _extent = old._extent;
        _format = old._format;
        _presentMode = old._presentMode;

        old._surface = nullptr;
        old._swapchain = {};
        old._oldSwapchain = nullptr;
        return *this;
    }


    auto VulkanSwapchain::resize(glm::uvec2 extent) noexcept -> std::optional<Error>
    {
        _extent = extent;
        const vkb::Swapchain swapchain = _swapchain;
        _oldSwapchain = swapchain.swapchain;
        std::optional<Error> result = createSwapchain();
        if (result)
        {
            return result;
        }

        _context.get().waitIdle();
        vkb::destroy_swapchain(swapchain);

        return std::nullopt;
    }

    auto VulkanSwapchain::acquireNextImage(Queue& queue) noexcept -> std::optional<Error>
    {
        const VulkanQueue* vulkanQueue = queue.as<VulkanQueue>();
        const vk::SwapchainKHR swapchain = _swapchain.swapchain;
        vk::ResultValue<uint32_t> const result = _context.get().getDevice()
                                                         .acquireNextImageKHR(swapchain,
                                                             std::numeric_limits<uint64_t>::max(),
                                                             vulkanQueue->
                                                             getCurrentPresentSemaphore(),
                                                             nullptr);
        if (result.result == vk::Result::eErrorOutOfDateKHR
            || result.result == vk::Result::eSuboptimalKHR)
        {
            return ErrorCode::eSwapchainNeedsResize;
        }
        if (result.result != vk::Result::eSuccess)
        {
            return ErrorCode::eSwapchainAcquireNextImageFailed;
        }

        _currentImage = result.value;
        return std::nullopt;
    }

    std::optional<Error> VulkanSwapchain::drawImage(CommandBuffer& commandBuffer,
                                                    Texture& texture) noexcept
    {
        const auto* vulkanTexture = texture.as<VulkanTexture>();
        const auto* vulkanCommandBuffer = commandBuffer.as<VulkanCommandBuffer>();

        // TODO: Implement this
        return std::nullopt;
    }

    auto VulkanSwapchain::getImage(uint32_t index) const noexcept -> vk::Image
    {
        return _swapchainImages[index];
    }
} // namespace exage::Graphics
