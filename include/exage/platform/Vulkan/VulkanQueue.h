#pragma once

#define VK_NO_PROTOTYPES
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_STORAGE_SHARED_EXPORT

#include "exage/Graphics/Queue.h"
#include "vulkan/vulkan.hpp"

namespace exage::Graphics
{
    class VulkanContext;
    
    struct VulkanQueueCreateInfo
    {
        size_t maxFramesInFlight;
        vk::Queue queue;
        uint32_t familyIndex;
    };

    class EXAGE_EXPORT VulkanQueue final : public Queue
    {
      public:
        VulkanQueue(VulkanContext& context, const VulkanQueueCreateInfo& createInfo) noexcept;
        ~VulkanQueue() override;

        EXAGE_DELETE_COPY(VulkanQueue);
        VulkanQueue(VulkanQueue&& old) noexcept;
        auto operator=(VulkanQueue&& old) noexcept -> VulkanQueue&;

        void startNextFrame() noexcept override;
        void submit(QueueSubmitInfo& submitInfo) noexcept override;
        [[nodiscard]] auto present(QueuePresentInfo& presentInfo) noexcept
            -> std::optional<Error> override;

        void submitTemporary(std::unique_ptr<CommandBuffer> commandBuffer) noexcept override;

        [[nodiscard]] auto currentFrame() const noexcept -> size_t override;
        [[nodiscard]] auto getFramesInFlight() const noexcept -> size_t override;

        [[nodiscard]] auto getCurrentPresentSemaphore() const noexcept -> vk::Semaphore;
        [[nodiscard]] auto getCurrentRenderSemaphore() const noexcept -> vk::Semaphore;
        [[nodiscard]] auto getCurrentFence() const noexcept -> vk::Fence;
        [[nodiscard]] auto getVulkanQueue() const noexcept -> vk::Queue { return _queue; }
        [[nodiscard]] auto getFamilyIndex() const noexcept -> uint32_t { return _familyIndex; }

        EXAGE_VULKAN_DERIVED;

      private:
        void cleanup() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        size_t _framesInFlight;
        vk::Queue _queue;
        uint32_t _familyIndex;

        std::vector<vk::Semaphore> _presentSemaphores;
        std::vector<vk::Semaphore> _renderSemaphores;
        std::vector<vk::Fence> _renderFences;

        size_t _currentFrame = 0;
    };
}  // namespace exage::Graphics
