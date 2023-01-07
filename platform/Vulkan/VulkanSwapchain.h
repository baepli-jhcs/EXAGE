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
        VulkanSwapchain(VulkanContext& context, const SwapchainCreateInfo& createInfo);
        ~VulkanSwapchain() override;

        EXAGE_DEFAULT_MOVE(VulkanSwapchain);
        EXAGE_DELETE_COPY(VulkanSwapchain);

        auto getPresentMode() const -> PresentMode override { return _presentMode; }

        auto getSwapchain() const -> vk::SwapchainKHR;
        auto getImage(uint32_t index) const -> vk::Image;

        void resize(glm::uvec2 extent) override;

      private:
        VulkanContext& _context;
        Window& _window;

        vk::SurfaceKHR _surface;
        Texture::Format _format;
        PresentMode _presentMode;
        vk::ColorSpaceKHR _colorSpace;

        vk::SwapchainKHR _swapchain;
        vk::SwapchainKHR _oldSwapchain;

        glm::uvec2 _extent;
        std::vector<vk::Image> _swapchainImages;

        std::vector<vk::Semaphore> _imageAvailableSemaphores;
        std::vector<vk::Semaphore> _renderFinishedSemaphores;
        std::vector<vk::Fence> _inFlightFences;
        std::vector<vk::Fence> _imagesInFlight;

        size_t _currentFrame = 0;
        size_t _currentImage = 0;
    };

}  // namespace exage::Graphics
