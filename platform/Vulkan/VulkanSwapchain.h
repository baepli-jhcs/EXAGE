#pragma once

#include "Core/Core.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Texture.h"
#include "Vulkan/VulkanContext.h"
#include "utils/classes.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanSwapchain final : public Swapchain
    {
    public:
        [[nodiscard]] static auto create(VulkanContext& context,
                                         const SwapchainCreateInfo& createInfo) noexcept
        -> tl::expected<VulkanSwapchain, Error>;
        ~VulkanSwapchain() override;

        EXAGE_DELETE_COPY(VulkanSwapchain);

        VulkanSwapchain(VulkanSwapchain&& old) noexcept;
        auto operator=(VulkanSwapchain&& old) noexcept -> VulkanSwapchain&;

        [[nodiscard]] auto getPresentMode() const noexcept -> PresentMode override
        {
            return _presentMode;
        }

        [[nodiscard]] auto resize(glm::uvec2 extent) noexcept -> std::optional<Error> override;
        [[nodiscard]] auto acquireNextImage(Queue& queue) noexcept -> std::optional<Error> override;
        [[nodiscard]] auto drawImage(CommandBuffer& commandBuffer,
                                     Texture& texture) noexcept -> std::optional<Error> override;

        [[nodiscard]] auto getSwapchain() const noexcept -> vk::SwapchainKHR
        {
            return _swapchain.swapchain;
        }

        [[nodiscard]] auto getImage(uint32_t index) const noexcept -> vk::Image;
        [[nodiscard]] auto getCurrentImage() const noexcept -> size_t { return _currentImage; }


        EXAGE_VULKAN_DERIVED;

    private:
        VulkanSwapchain(VulkanContext& context, const SwapchainCreateInfo& createInfo) noexcept;
        [[nodiscard]] std::optional<Error> init(Window& window) noexcept;
        [[nodiscard]] std::optional<Error> createSurface(Window& window) noexcept;
        [[nodiscard]] std::optional<Error> createSwapchain() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vk::SurfaceKHR _surface = nullptr;
        Texture::Format _format = Texture::Format::eRGBA8;
        Texture::Layout _layout = Texture::Layout::eUndefined;
        vk::Format _vkFormat = vk::Format::eR8G8B8A8Unorm;
        PresentMode _presentMode;
        vk::ColorSpaceKHR _colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

        vkb::Swapchain _swapchain;
        vk::SwapchainKHR _oldSwapchain = nullptr;

        glm::uvec2 _extent;
        std::vector<vk::Image> _swapchainImages;
        std::vector<bool> _swapchainTransitioned;

        size_t _currentImage = 0;
    };
} // namespace exage::Graphics
