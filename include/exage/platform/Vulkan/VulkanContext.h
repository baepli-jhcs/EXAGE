#pragma once

#include "exage/Graphics/Context.h"

#define VK_NO_PROTOTYPES
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_STORAGE_SHARED_EXPORT

#include <optional>

#include <VkBootstrap.h>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "exage/Graphics/Pipeline.h"
#include "exage/Graphics/Queue.h"
#include "exage/platform/Vulkan/VulkanQueue.h"
#include "exage/platform/Vulkan/VulkanUtils.h"

namespace exage::Graphics
{
    enum class SurfaceError : uint32_t;

    class EXAGE_EXPORT VulkanContext final : public Context
    {
      public:
        [[nodiscard]] static tl::expected<std::unique_ptr<VulkanContext>, Error> create(
            ContextCreateInfo& createInfo) noexcept;
        ~VulkanContext() override;

        EXAGE_DELETE_COPY(VulkanContext);
        EXAGE_DELETE_MOVE(VulkanContext);

        struct PipelineLayoutInfo;

        void waitIdle() const noexcept override;

        [[nodiscard]] auto getQueue() noexcept -> Queue& override { return *_queue; }
        [[nodiscard]] auto getQueue() const noexcept -> const Queue& override { return *_queue; }

        [[nodiscard]] auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
            -> std::unique_ptr<Swapchain> override;
        [[nodiscard]] auto createCommandBuffer() noexcept
            -> std::unique_ptr<CommandBuffer> override;
        [[nodiscard]] auto createTexture(const TextureCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Texture> override;
        [[nodiscard]] auto createFrameBuffer(glm::uvec2 extent) noexcept
            -> std::shared_ptr<FrameBuffer> override;
        [[nodiscard]] auto createBuffer(const BufferCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Buffer> override;
        [[nodiscard]] auto createShader(const ShaderCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Shader> override;
		[[nodiscard]] auto createPipeline(const PipelineCreateInfo& createInfo) noexcept
			-> std::shared_ptr<Pipeline> override;
        [[nodiscard]] auto createResourceManager() noexcept
			-> std::unique_ptr<ResourceManager> override;

        [[nodiscard]] auto getHardwareSupport() const noexcept -> HardwareSupport override;

        [[nodiscard]] auto createSurface(Window& window) const noexcept -> vk::SurfaceKHR;
        [[nodiscard]] auto getOrCreateDescriptorSetLayout(
            const std::vector<ResourceDescription>& resourceDescriptions) noexcept
            -> vk::DescriptorSetLayout;
        [[nodiscard]] auto getOrCreatePipelineLayout(const PipelineLayoutInfo& info) noexcept
            -> vk::PipelineLayout;

        [[nodiscard]] auto getInstance() const noexcept -> vk::Instance;
        [[nodiscard]] auto getPhysicalDevice() const noexcept -> vk::PhysicalDevice;
        [[nodiscard]] auto getDevice() const noexcept -> vk::Device;
        [[nodiscard]] auto getAllocator() const noexcept -> vma::Allocator;
        [[nodiscard]] auto getVulkanBootstrapDevice() const noexcept -> vkb::Device;

        [[nodiscard]] auto getVulkanQueue() noexcept -> VulkanQueue& { return *_queue; }
        [[nodiscard]] auto getVulkanQueue() const noexcept -> const VulkanQueue& { return *_queue; }

        EXAGE_VULKAN_DERIVED

        struct PipelineLayoutInfo
        {
            std::vector<ResourceDescription> resourceDescriptions;
            uint32_t pushConstantSize;
        };

      private:
        VulkanContext() = default;
        auto init(ContextCreateInfo& createInfo) noexcept -> std::optional<Error>;

        vma::Allocator _allocator;
        vkb::Instance _instance;
        vkb::PhysicalDevice _physicalDevice;
        vkb::Device _device;
        std::optional<VulkanQueue> _queue = std::nullopt;

        HardwareSupport _hardwareSupport;

        std::unordered_map<size_t, vk::DescriptorSetLayout> _descriptorSetLayoutCache;
        std::unordered_map<size_t, vk::PipelineLayout> _pipelineLayoutCache;
    };
}  // namespace exage::Graphics
