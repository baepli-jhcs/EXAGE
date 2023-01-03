#pragma once

#define VK_NO_PROTOTYPES
#include <VkBootstrap.h>
#include <vulkan/vulkan.hpp>

#include "Graphics/Context.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanContext final : public Context
    {
      public:
        explicit VulkanContext(WindowAPI windowAPI);
        ~VulkanContext() override;

        EXAGE_DELETE_COPY(VulkanContext);
        EXAGE_DEFAULT_MOVE(VulkanContext);

        auto createSwapchain(Window& window) -> Swapchain* override;

        auto getAPI() const -> API override { return API::eVulkan; }

        auto getInstance() const -> vk::Instance;
        auto getPhysicalDevice() const -> vk::PhysicalDevice;
        auto getDevice() const -> vk::Device;

        auto getGraphicsQueue() const -> vk::Queue;
        auto getPresentQueue() const -> vk::Queue;
        auto getComputeQueue() const -> vk::Queue;

      private:
        vkb::Instance _instance;
        vkb::PhysicalDevice _physicalDevice;
        vkb::Device _device;
        vk::Queue _graphicsQueue;
        vk::Queue _presentQueue;
        vk::Queue _computeQueue;
    };
}  // namespace exage::Graphics