#pragma once

#include "exage/Graphics/Queue.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/platform/Vulkan/VKinclude.h"

namespace exage::Graphics
{
    class VulkanContext;

    struct VulkanQueueCreateInfo
    {
        uint32_t maxFramesInFlight;
        vk::Queue queue;
        uint32_t familyIndex;
    };

    class VulkanQueue final : public Queue
    {
      public:
        VulkanQueue(VulkanContext& context, const VulkanQueueCreateInfo& createInfo) noexcept;
        ~VulkanQueue() override;

        EXAGE_DELETE_COPY(VulkanQueue);
        VulkanQueue(VulkanQueue&& old) noexcept;
        auto operator=(VulkanQueue&& old) noexcept -> VulkanQueue&;

        void startNextFrame() noexcept override;
        void submit(CommandBuffer& commandBuffer) noexcept override;
        [[nodiscard]] auto present(Swapchain& swapchain) noexcept
            -> tl::expected<void, Error> override;

        void submitTemporary(std::unique_ptr<CommandBuffer> commandBuffer) noexcept override;

        [[nodiscard]] auto currentFrame() const noexcept -> uint32_t override;
        [[nodiscard]] auto getFramesInFlight() const noexcept -> uint32_t override;

        [[nodiscard]] auto getCurrentPresentSemaphore() const noexcept -> vk::Semaphore;
        [[nodiscard]] auto getCurrentRenderSemaphore() const noexcept -> vk::Semaphore;
        [[nodiscard]] auto getCurrentFence() const noexcept -> vk::Fence;
        [[nodiscard]] auto getVulkanQueue() const noexcept -> vk::Queue { return _queue; }
        [[nodiscard]] auto getFamilyIndex() const noexcept -> uint32_t { return _familyIndex; }

        EXAGE_VULKAN_DERIVED;

      private:
        void cleanup() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        uint32_t _framesInFlight;
        vk::Queue _queue;
        uint32_t _familyIndex;

        std::vector<vk::Semaphore> _presentSemaphores;
        std::vector<vk::Semaphore> _renderSemaphores;
        std::vector<vk::Fence> _renderFences;

        uint32_t _currentFrame = 0;
    };

    struct VulkanTransferQueueCreateInfo
    {
        vk::Queue queue;
        uint32_t familyIndex;
    };

    class VulkanTransferQueue final : public TransferQueue
    {
      public:
        VulkanTransferQueue(VulkanContext& context,
                            const VulkanTransferQueueCreateInfo& createInfo) noexcept;
        ~VulkanTransferQueue() override;

        EXAGE_DELETE_COPY(VulkanTransferQueue);
        VulkanTransferQueue(VulkanTransferQueue&& old) noexcept;
        auto operator=(VulkanTransferQueue&& old) noexcept -> VulkanTransferQueue&;

        void submit(CommandBuffer& commandBuffer, Fence* fence) noexcept override;

        [[nodiscard]] auto getVulkanQueue() const noexcept -> vk::Queue { return _queue; }
        [[nodiscard]] auto getFamilyIndex() const noexcept -> uint32_t { return _familyIndex; }

        EXAGE_VULKAN_DERIVED;

      private:
        void cleanup() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vk::Queue _queue;
        uint32_t _familyIndex;
    };
}  // namespace exage::Graphics
