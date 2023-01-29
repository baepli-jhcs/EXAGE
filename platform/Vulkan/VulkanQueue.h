#pragma once

#include "Graphics/Queue.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    struct VulkanQueueCreateInfo
    {
        size_t maxFramesInFlight;
        vk::Queue queue;
    };

    class EXAGE_EXPORT VulkanQueue : public Queue
    {
    public:
        [[nodiscard]] static tl::expected<std::unique_ptr<Queue>, Error> create(
            VulkanContext& context,
            const VulkanQueueCreateInfo& createInfo) noexcept;
        ~VulkanQueue() override;

        EXAGE_DELETE_COPY(VulkanQueue);
        EXAGE_DEFAULT_MOVE(VulkanQueue);

        [[nodiscard]] auto startNextFrame() noexcept -> std::optional<Error> override;
        [[nodiscard]] auto submit(SwapchainSubmits& submits) noexcept ->
        std::optional<Error> override;

        [[nodiscard]] auto currentFrame() const noexcept -> size_t override;

        EXAGE_VULKAN_DERIVED;

    private:
        explicit VulkanQueue(
            VulkanContext& context,
            const VulkanQueueCreateInfo& createInfo) noexcept;
        std::optional<Error> init() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        size_t _framesInFlight;
        vk::Queue _queue;

        std::vector<vk::Semaphore> _presentSemaphores;
        std::vector<vk::Semaphore> _renderSemaphores;
        std::vector<vk::Fence> _renderFences;

        size_t _currentFrame = 0;
    };
} // namespace exage::Graphics
