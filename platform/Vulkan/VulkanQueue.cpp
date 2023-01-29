#include "VulkanQueue.h"

namespace exage::Graphics
{
    tl::expected<std::unique_ptr<Queue>, Error> VulkanQueue::create(VulkanContext& context,
        const VulkanQueueCreateInfo& createInfo) noexcept
    {
        auto queue = std::unique_ptr<VulkanQueue>(new VulkanQueue(context, createInfo));
        if (auto error = queue->init(); error.has_value())
        {
            return tl::make_unexpected(error.value());
        }
        return queue;
    }

    VulkanQueue::VulkanQueue(VulkanContext& context,
                             const VulkanQueueCreateInfo& createInfo) noexcept
        : _context(context),
          _framesInFlight(createInfo.maxFramesInFlight)
          , _queue(createInfo.queue) {}

    std::optional<Error> VulkanQueue::init() noexcept
    {
        _renderFences.resize(_framesInFlight);
        _presentSemaphores.resize(_framesInFlight);
        _renderSemaphores.resize(_framesInFlight);

        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        vk::SemaphoreCreateInfo semaphoreCreateInfo;

        for (size_t i = 0; i < _framesInFlight; i++)
        {
            vk::Result fenceResult = _context.get().getDevice().createFence(&fenceCreateInfo,
                nullptr,
                &_renderFences[i]);

            if (fenceResult != vk::Result::eSuccess)
            {
                return ErrorCode::eFenceCreationFailed;
            }

            vk::Result semaphoreResult = _context.get().getDevice().createSemaphore(
                &semaphoreCreateInfo,
                nullptr,
                &_presentSemaphores[i]);

            if (semaphoreResult != vk::Result::eSuccess)
            {
                return ErrorCode::eSemaphoreCreationFailed;
            }

            semaphoreResult = _context.get().getDevice().createSemaphore(
                &semaphoreCreateInfo,
                nullptr,
                &_renderSemaphores[i]);

            if (semaphoreResult != vk::Result::eSuccess)
            {
                return ErrorCode::eSemaphoreCreationFailed;
            }
        }
    }

    VulkanQueue::~VulkanQueue()
    {
        _context.get().waitIdle();

        for (const auto& semaphore : _presentSemaphores)
        {
            _context.get().getDevice().destroySemaphore(semaphore);
        }

        for (const auto& semaphore : _renderSemaphores)
        {
            _context.get().getDevice().destroySemaphore(semaphore);
        }

        for (const auto& fence : _renderFences)
        {
            _context.get().getDevice().destroyFence(fence);
        }
    }

    auto VulkanQueue::startNextFrame() noexcept -> std::optional<Error>
    {
        _currentFrame = (_currentFrame + 1) % _framesInFlight;

        vk::Result result = _context.get().getDevice().waitForFences(
            1,
            &_renderFences[_currentFrame],
            true,
            std::numeric_limits<uint64_t>::max());

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eFenceWaitFailed;
        }

        result = _context.get().getDevice().resetFences(1, &_renderFences[_currentFrame]);

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eFenceResetFailed;
        }

        return std::nullopt;
    }

    auto VulkanQueue::submit(SwapchainSubmits& submits) noexcept -> std::optional<Error> { }
}
