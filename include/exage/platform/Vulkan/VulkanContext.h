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

        [[nodiscard]] auto getTransferQueue() noexcept -> TransferQueue& override
        {
            return *_transferQueue;
        }
        [[nodiscard]] auto getTransferQueue() const noexcept -> const TransferQueue& override
        {
            return *_transferQueue;
        }

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
        [[nodiscard]] auto createGraphicsPipeline(
            const GraphicsPipelineCreateInfo& createInfo) noexcept
            -> std::shared_ptr<GraphicsPipeline> override;
        [[nodiscard]] auto createFence() noexcept -> std::unique_ptr<Fence> override;

        [[nodiscard]] auto getHardwareSupport() const noexcept -> HardwareSupport override;
        [[nodiscard]] auto getFormatSupport(Format format) const noexcept
            -> std::pair<bool, FormatFeatures> override;

        [[nodiscard]] auto createSurface(System::Window& window) const noexcept -> vk::SurfaceKHR;
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

        [[nodiscard]] auto getVulkanTransferQueue() noexcept -> VulkanTransferQueue&
        {
            return *_transferQueue;
        }
        [[nodiscard]] auto getVulkanTransferQueue() const noexcept -> const VulkanTransferQueue&
        {
            return *_transferQueue;
        }

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

        [[nodiscard]] auto getCommandPoolMutex() noexcept -> std::mutex&
        {
            return _commandPoolMutex;
        }

        [[nodiscard]] auto createVulkanCommandBuffer() noexcept -> vk::CommandBuffer;
        void destroyCommandBuffer(vk::CommandBuffer commandBuffer) noexcept;

        void processDeletions(uint32_t frameIndex) noexcept;

        void destroyBuffer(vk::Buffer buffer) noexcept;
        void destroyImage(vk::Image image) noexcept;
        void destroyImageView(vk::ImageView imageView) noexcept;
        void destroySampler(vk::Sampler sampler) noexcept;
        void destroyAllocation(vma::Allocation allocation) noexcept;
        void destroySwapchain(vkb::Swapchain swapchain) noexcept;
        void destroyPipeline(vk::Pipeline pipeline) noexcept;
        void destroyPipelineLayout(vk::PipelineLayout pipelineLayout) noexcept;
        void destroyBufferID(BufferID bufferID) noexcept;
        void destroyTextureID(TextureID textureID) noexcept;
        void destroySamplerID(SamplerID samplerID) noexcept;

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
        std::optional<VulkanTransferQueue> _transferQueue = std::nullopt;
        std::optional<VulkanResourceManager> _resourceManager = std::nullopt;

        std::mutex _commandPoolMutex;
        vk::CommandPool _commandPool;

        HardwareSupport _hardwareSupport;

        std::mutex _descriptorSetLayoutCacheMutex;
        std::unordered_map<size_t, vk::DescriptorSetLayout> _descriptorSetLayoutCache;
        std::mutex _pipelineLayoutCacheMutex;
        std::unordered_map<size_t, vk::PipelineLayout> _pipelineLayoutCache;

        std::vector<vk::CommandBuffer> _freeCommandBuffers;

        template<typename T>
        struct DeletionQueue
        {
            std::mutex mutex {};
            std::array<std::vector<T>, MAX_FRAMES_IN_FLIGHT> deletions {};

            void push(VulkanContext& context, T object) noexcept
            {
                std::lock_guard<std::mutex> lock(mutex);
                deletions[context._queue->currentFrame()].push_back(object);
            }

            template<typename F>
            void process(uint32_t frameIndex, F&& function) noexcept
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& object : deletions[frameIndex])
                {
                    function(object);
                }
                deletions[frameIndex].clear();
            }
        };

        DeletionQueue<vk::Buffer> _bufferDeletionQueue;
        DeletionQueue<vk::Image> _imageDeletionQueue;
        DeletionQueue<vk::ImageView> _imageViewDeletionQueue;
        DeletionQueue<vk::Sampler> _samplerDeletionQueue;
        DeletionQueue<vma::Allocation> _allocationDeletionQueue;
        DeletionQueue<vkb::Swapchain> _swapchainDeletionQueue;
        DeletionQueue<vk::Pipeline> _pipelineDeletionQueue;
        DeletionQueue<vk::PipelineLayout> _pipelineLayoutDeletionQueue;
        DeletionQueue<BufferID> _bufferIDDeletionQueue;
        DeletionQueue<TextureID> _textureIDDeletionQueue;
        DeletionQueue<SamplerID> _samplerIDDeletionQueue;
    };
}  // namespace exage::Graphics
