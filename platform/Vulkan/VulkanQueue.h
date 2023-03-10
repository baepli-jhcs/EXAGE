#pragma once

#include "Graphics/Queue.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanQueue final : public Queue
    {
    public:
        [[nodiscard]] static tl::expected<VulkanQueue, Error> create(
            VulkanContext& context,
            const QueueCreateInfo& createInfo) noexcept;
        ~VulkanQueue() override;

        EXAGE_DELETE_COPY(VulkanQueue);
        VulkanQueue(VulkanQueue&& old) noexcept;
        auto operator=(VulkanQueue&& old) noexcept -> VulkanQueue&;

        [[nodiscard]] auto startNextFrame() noexcept -> std::optional<Error> override;
        [[nodiscard]] auto submit(QueueSubmitInfo& submitInfo) noexcept ->
        std::optional<Error> override;
        [[nodiscard]] auto present(QueuePresentInfo& presentInfo) noexcept ->
        std::optional<Error> override;
        
        [[nodiscard]] auto submitTemporary(
            std::unique_ptr<CommandBuffer> commandBuffer) -> std::optional<Error> override;

        [[nodiscard]] auto currentFrame() const noexcept -> size_t override;
        [[nodiscard]] auto getFramesInFlight() const noexcept -> size_t override;

        [[nodiscard]] auto getCurrentPresentSemaphore() const noexcept -> vk::Semaphore;
        [[nodiscard]] auto getCurrentRenderSemaphore() const noexcept -> vk::Semaphore;
        [[nodiscard]] auto getCurrentFence() const noexcept -> vk::Fence;

        EXAGE_VULKAN_DERIVED;

    private:
        explicit VulkanQueue(
            VulkanContext& context,
            const QueueCreateInfo& createInfo) noexcept;
        std::optional<Error> init() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        size_t _framesInFlight;

        std::vector<vk::Semaphore> _presentSemaphores;
        std::vector<vk::Semaphore> _renderSemaphores;
        std::vector<vk::Fence> _renderFences;

        size_t _currentFrame = 0;
    };
} // namespace exage::Graphics
