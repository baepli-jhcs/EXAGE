#pragma once

#include "Graphics/Context.h"

#define VK_NO_PROTOTYPES
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <optional>

#include <VkBootstrap.h>
#include <vulkan/vulkan.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

namespace exage::Graphics
{
    enum class SurfaceError : uint32_t;


    class EXAGE_EXPORT VulkanContext final : public Context
    {
    public:
        [[nodiscard]] static tl::expected<std::unique_ptr<Context>, Error> create(
            ContextCreateInfo& createInfo) noexcept;
        ~VulkanContext() override;

        EXAGE_DELETE_COPY(VulkanContext);
        EXAGE_DEFAULT_MOVE(VulkanContext);

        void waitIdle() const noexcept override;

        [[nodiscard]] auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<Swapchain>, Error> override;

        [[nodiscard]] auto createSurface(
            Window& window) const noexcept -> tl::expected<vk::SurfaceKHR, Error>;

        [[nodiscard]] auto getQueue() noexcept -> Queue& override;
        [[nodiscard]] auto getQueue() const noexcept -> const Queue& override;

        [[nodiscard]] auto getInstance() const noexcept -> vk::Instance;
        [[nodiscard]] auto getPhysicalDevice() const noexcept -> vk::PhysicalDevice;
        [[nodiscard]] auto getDevice() const noexcept -> vk::Device;
        [[nodiscard]] auto getAllocator() const noexcept -> vma::Allocator;

        [[nodiscard]] auto getVulkanBootstrapDevice() const noexcept -> vkb::Device
        {
            return _device;
        }

        [[nodiscard]] auto getGraphicsQueue() const noexcept -> vk::Queue;

        EXAGE_VULKAN_DERIVED

    private:
        VulkanContext() = default;
        auto init(ContextCreateInfo& createInfo) noexcept -> std::optional<Error>;

        vkb::Instance _instance;
        vkb::PhysicalDevice _physicalDevice;
        vkb::Device _device;

        vma::Allocator _allocator;

        std::unique_ptr<Queue> _queue;
    };
} // namespace exage::Graphics
