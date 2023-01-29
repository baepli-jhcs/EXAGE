#include "VulkanSwapchain.h"

namespace exage::Graphics
{
    VulkanSwapchain::VulkanSwapchain(VulkanContext& context,
                                     const SwapchainCreateInfo& createInfo) noexcept
        : _context(context)
          , _presentMode(createInfo.presentMode), _extent(createInfo.window.getExtent()) { }


    constexpr auto DESIRED_FORMAT = toVulkanTextureFormat(Texture::Format::eRGBA8);
    constexpr auto FALLBACK_FORMAT = toVulkanTextureFormat(Texture::Format::eBGRA8);

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
                                 const SwapchainCreateInfo& createInfo) noexcept -> tl::expected<
        std::unique_ptr<VulkanSwapchain>,
        Error>
    {
        std::unique_ptr<VulkanSwapchain> swapchain{new VulkanSwapchain(context, createInfo)};
        std::optional<Error> result = swapchain->init(createInfo.window);
        if (result)
        {
            return tl::make_unexpected(*result);
        }

        return swapchain;
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        _context.get().waitIdle();

        vkb::destroy_swapchain(_swapchain);
        vkb::destroy_surface(_context.get().getInstance(), _surface);
    }

    auto VulkanSwapchain::resize(glm::uvec2 extent) noexcept -> std::optional<Error>
    {
        _extent = extent;
        const vkb::Swapchain swapchain = std::move(_swapchain);
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

    auto VulkanSwapchain::acquireNextImage() noexcept -> std::optional<Error>
    {
        const vk::SwapchainKHR swapchain = _swapchain.swapchain;
        vk::ResultValue<uint32_t> const result = _context.get().getDevice()
                                                         .acquireNextImageKHR(swapchain,
                                                             std::numeric_limits<uint64_t>::max(),
                                                             nullptr,
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

    auto VulkanSwapchain::getImage(uint32_t index) const noexcept -> vk::Image
    {
        return _swapchainImages[index];
    }
} // namespace exage::Graphics
