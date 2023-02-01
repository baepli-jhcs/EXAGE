#pragma once

#include "Core/Core.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Texture.h"
#include "Vulkan/VulkanContext.h"
#include "utils/classes.h"

namespace exage::Graphics
{
    [[nodiscard]] constexpr auto toVulkanPresentMode(
        PresentMode presentMode) noexcept -> vk::PresentModeKHR
    {
        switch (presentMode)
        {
            case PresentMode::eImmediate:
                return vk::PresentModeKHR::eImmediate;

            case PresentMode::eDoubleBufferVSync:
                return vk::PresentModeKHR::eFifo;

            case PresentMode::eTripleBufferVSync:
                return vk::PresentModeKHR::eMailbox;

            default:
                return vk::PresentModeKHR::eFifo;
        }
    }


    class EXAGE_EXPORT VulkanSwapchain final : public Swapchain
    {
    public:
        [[nodiscard]] static auto create(VulkanContext& context,
                                         const SwapchainCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<VulkanSwapchain>, Error>;
        ~VulkanSwapchain() override;

        EXAGE_DEFAULT_MOVE(VulkanSwapchain);
        EXAGE_DELETE_COPY(VulkanSwapchain);

        [[nodiscard]] auto getPresentMode() const noexcept -> PresentMode override
        {
            return _presentMode;
        }

        [[nodiscard]] auto resize(glm::uvec2 extent) noexcept -> std::optional<Error> override;
        [[nodiscard]] auto acquireNextImage(Queue& queue) noexcept -> std::optional<Error> override;

        [[nodiscard]] auto getSwapchain() const noexcept -> vk::SwapchainKHR
        {
            return _swapchain.swapchain;
        }

        [[nodiscard]] auto getImage(uint32_t index) const noexcept -> vk::Image;


        EXAGE_VULKAN_DERIVED;

    private:
        VulkanSwapchain(VulkanContext& context, const SwapchainCreateInfo& createInfo) noexcept;
        [[nodiscard]] std::optional<Error> init(Window& window) noexcept;
        [[nodiscard]] std::optional<Error> createSurface(Window& window) noexcept;
        [[nodiscard]] std::optional<Error> createSwapchain() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vk::SurfaceKHR _surface = nullptr;
        Texture::Format _format = Texture::Format::eRGBA8;
        vk::Format _vkFormat = vk::Format::eR8G8B8A8Unorm;
        PresentMode _presentMode;
        vk::ColorSpaceKHR _colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

        vkb::Swapchain _swapchain;
        vk::SwapchainKHR _oldSwapchain = nullptr;

        glm::uvec2 _extent;
        std::vector<vk::Image> _swapchainImages;

        size_t _currentImage = 0;
    };

    [[nodiscard]] constexpr auto toVulkanTextureFormat(Texture::Format format) noexcept
    -> vk::Format
    {
        switch (format)
        {
            case Texture::Format::eR8:
                return vk::Format::eR8Unorm;
            case Texture::Format::eR16:
                return vk::Format::eR16Unorm;
            case Texture::Format::eRG8:
                return vk::Format::eR8G8Unorm;
            case Texture::Format::eRG16:
                return vk::Format::eR16G16Unorm;
            case Texture::Format::eRGB8:
                return vk::Format::eR8G8B8Unorm;
            case Texture::Format::eRGB16:
                return vk::Format::eR16G16B16Unorm;
            case Texture::Format::eRGBA8:
                return vk::Format::eR8G8B8A8Unorm;
            case Texture::Format::eRGBA16:
                return vk::Format::eR16G16B16A16Unorm;

            case Texture::Format::eR16f:
                return vk::Format::eR16Sfloat;
            case Texture::Format::eRG16f:
                return vk::Format::eR16G16Sfloat;
            case Texture::Format::eRGB16f:
                return vk::Format::eR16G16B16Sfloat;
            case Texture::Format::eRGBA16f:
                return vk::Format::eR16G16B16A16Sfloat;

            case Texture::Format::eR32f:
                return vk::Format::eR32Sfloat;
            case Texture::Format::eRG32f:
                return vk::Format::eR32G32Sfloat;
            case Texture::Format::eRGB32f:
                return vk::Format::eR32G32B32Sfloat;
            case Texture::Format::eRGBA32f:
                return vk::Format::eR32G32B32A32Sfloat;

            case Texture::Format::eDepth24Stencil8:
                return vk::Format::eD24UnormS8Uint;
            case Texture::Format::eDepth32Stencil8:
                return vk::Format::eD32SfloatS8Uint;

            case Texture::Format::eBGRA8:
                return vk::Format::eB8G8R8A8Unorm;

            default:
                return vk::Format::eUndefined;
        }
    }
} // namespace exage::Graphics
