#pragma once

#include <optional>

#include <exage/platform/Vulkan/VKinclude.h>

#include "exage/Graphics/Context.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Graphics/Queue.h"
#include "exage/platform/Vulkan/VkBootstrap.h"
#include "exage/platform/Vulkan/VulkanQueue.h"
#include "exage/platform/Vulkan/VulkanResourceManager.h"
#include "exage/platform/Vulkan/VulkanUtils.h"

namespace exage::Graphics
{
    class VulkanResourceManager;

    class VulkanContext final : public Context
    {
      public:
        [[nodiscard]] static auto create(ContextCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<VulkanContext>, Error>;
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
        [[nodiscard]] auto createSampler(const SamplerCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Sampler> override;
        [[nodiscard]] auto createTexture(const TextureCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Texture> override;
        [[nodiscard]] auto createFrameBuffer(glm::uvec2 extent) noexcept
            -> std::shared_ptr<FrameBuffer> override;
        [[nodiscard]] auto createFrameBuffer(const FrameBufferCreateInfo& createInfo) noexcept
            -> std::shared_ptr<FrameBuffer> override;
        [[nodiscard]] auto createBuffer(const BufferCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Buffer> override;
        [[nodiscard]] auto createShader(const ShaderCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Shader> override;
        [[nodiscard]] auto createPipeline(const PipelineCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Pipeline> override;

        [[nodiscard]] auto getHardwareSupport() const noexcept -> HardwareSupport override;
        [[nodiscard]] auto getFormatSupport(Format format) const noexcept
            -> std::pair<bool, FormatFeatures> override;

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

        [[nodiscard]] auto getResourceManager() noexcept -> VulkanResourceManager&
        {
            return *_resourceManager;
        }
        [[nodiscard]] auto getResourceManager() const noexcept -> const VulkanResourceManager&
        {
            return *_resourceManager;
        }

        [[nodiscard]] auto getCommandPool() const noexcept -> vk::CommandPool
        {
            return _commandPool;
        }

        [[nodiscard]] auto createVulkanCommandBuffer() noexcept -> vk::CommandBuffer;
        void destroyCommandBuffer(vk::CommandBuffer commandBuffer) noexcept;

        EXAGE_VULKAN_DERIVED

        struct PipelineLayoutInfo
        {
            std::vector<ResourceDescription> resourceDescriptions;
            uint32_t pushConstantSize;
            bool bindless;
        };

      private:
        VulkanContext()
            : Context({.depthZeroToOne = true}) {};
        auto init(ContextCreateInfo& createInfo) noexcept -> tl::expected<void, Error>;

        vma::Allocator _allocator;
        vkb::Instance _instance;
        vkb::PhysicalDevice _physicalDevice;
        vkb::Device _device;
        std::optional<VulkanQueue> _queue = std::nullopt;
        std::optional<VulkanResourceManager> _resourceManager = std::nullopt;
        vk::CommandPool _commandPool;

        HardwareSupport _hardwareSupport;

        std::unordered_map<size_t, vk::DescriptorSetLayout> _descriptorSetLayoutCache;
        std::unordered_map<size_t, vk::PipelineLayout> _pipelineLayoutCache;

        std::vector<vk::CommandBuffer> _freeCommandBuffers;
    };
}  // namespace exage::Graphics
