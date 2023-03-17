#pragma once

#include "Graphics/Context.h"

#define VK_NO_PROTOTYPES
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_STORAGE_SHARED_EXPORT

#include <optional>

#include <VkBootstrap.h>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "Graphics/Queue.h"
#include "VulkanUtils.h"

namespace exage::Graphics
{
    class VulkanQueue;
    enum class SurfaceError : uint32_t;

    class EXAGE_EXPORT VulkanContext final : public Context
    {
      public:
        [[nodiscard]] static tl::expected<VulkanContext, Error> create(
            ContextCreateInfo& createInfo) noexcept;
        ~VulkanContext() override;

        EXAGE_DELETE_COPY(VulkanContext);

        VulkanContext(VulkanContext&& old) noexcept;
        auto operator=(VulkanContext&& old) noexcept -> VulkanContext&;

        void waitIdle() const noexcept override;

        [[nodiscard]] auto createQueue(const QueueCreateInfo& createInfo) noexcept
            -> std::unique_ptr<Queue> override;
        [[nodiscard]] auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
            -> std::unique_ptr<Swapchain> override;
        [[nodiscard]] auto createCommandBuffer() noexcept
            -> std::unique_ptr<CommandBuffer> override;
        [[nodiscard]] auto createTexture(const TextureCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Texture> override;
        [[nodiscard]] auto createFrameBuffer(glm::uvec2 extent) noexcept
            -> std::shared_ptr<FrameBuffer> override;

        [[nodiscard]] auto createSurface(Window& window) const noexcept -> vk::SurfaceKHR;

        [[nodiscard]] auto getInstance() const noexcept -> vk::Instance;
        [[nodiscard]] auto getPhysicalDevice() const noexcept -> vk::PhysicalDevice;
        [[nodiscard]] auto getDevice() const noexcept -> vk::Device;
        [[nodiscard]] auto getAllocator() const noexcept -> vma::Allocator;

        [[nodiscard]] auto getQueueIndex() const noexcept -> uint32_t;
        [[nodiscard]] auto getVulkanQueue() const noexcept -> vk::Queue;

        [[nodiscard]] auto getVulkanBootstrapDevice() const noexcept -> vkb::Device;

        EXAGE_VULKAN_DERIVED

      private:
        VulkanContext() = default;
        auto init(ContextCreateInfo& createInfo) noexcept -> std::optional<Error>;

        vma::Allocator _allocator;
        vkb::Instance _instance;
        vkb::PhysicalDevice _physicalDevice;
        vkb::Device _device;
    };
}  // namespace exage::Graphics
