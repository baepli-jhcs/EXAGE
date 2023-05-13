#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/Graphics/Texture.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/utils/classes.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanSwapchain final : public Swapchain
    {
      public:
        VulkanSwapchain(VulkanContext& context, const SwapchainCreateInfo& createInfo) noexcept;
        ~VulkanSwapchain() override;

        EXAGE_DELETE_COPY(VulkanSwapchain);

        VulkanSwapchain(VulkanSwapchain&& old) noexcept;
        auto operator=(VulkanSwapchain&& old) noexcept -> VulkanSwapchain&;

        [[nodiscard]] auto getPresentMode() const noexcept -> PresentMode override
        {
            return _presentMode;
        }

        void resize(glm::uvec2 extent) noexcept override;
        [[nodiscard]] auto acquireNextImage() noexcept -> tl::expected<void, Error> override;
        void drawImage(CommandBuffer& commandBuffer,
                       const std::shared_ptr<Texture>& texture) noexcept override;

        [[nodiscard]] auto getSwapchain() const noexcept -> vk::SwapchainKHR
        {
            return _swapchain.swapchain;
        }

        [[nodiscard]] auto getImage(uint32_t index) const noexcept -> vk::Image;
        [[nodiscard]] auto getCurrentImage() const noexcept -> size_t { return _currentImage; }

        EXAGE_VULKAN_DERIVED;

      private:
        void createSurface(Window& window) noexcept;
        void createSwapchain() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vk::SurfaceKHR _surface = nullptr;
        Format _format = Format::eRGBA8;
        Texture::Layout _layout = Texture::Layout::eUndefined;
        vk::Format _vkFormat = vk::Format::eR8G8B8A8Unorm;
        PresentMode _presentMode;
        vk::ColorSpaceKHR _colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

        vkb::Swapchain _swapchain;
        vk::SwapchainKHR _oldSwapchain = nullptr;

        glm::uvec2 _extent {};
        std::vector<vk::Image> _swapchainImages;
        std::vector<bool> _swapchainTransitioned;

        size_t _currentImage = 0;
    };
}  // namespace exage::Graphics
