#include "VulkanQueue.h"

#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"

namespace exage::Graphics
{
    tl::expected<std::unique_ptr<VulkanQueue>, Error> VulkanQueue::create(
        VulkanContext& context,
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
        : _context(context)
          , _framesInFlight(createInfo.maxFramesInFlight)
          , _queue(createInfo.queue),
          _familyIndex(createInfo.familyIndex) { }

    std::optional<Error> VulkanQueue::init() noexcept
    {
        _renderFences.resize(_framesInFlight);
        _presentSemaphores.resize(_framesInFlight);
        _renderSemaphores.resize(_framesInFlight);

        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        vk::SemaphoreCreateInfo const semaphoreCreateInfo;

        for (size_t i = 0; i < _framesInFlight; i++)
        {
            vk::Result fenceResult = _context.get().getDevice().createFence(
                &fenceCreateInfo,
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

        return std::nullopt;
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

        vk::Result result =
            _context.get().getDevice().waitForFences(1,
                                                     &_renderFences[_currentFrame],
                                                     /*waitAll=*/
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

    auto VulkanQueue::submit(QueueSubmitInfo& submitInfo) noexcept -> std::optional<Error>
    {
        vk::SubmitInfo vkSubmitInfo;
        vkSubmitInfo.commandBufferCount = 1;

        auto* queueCommandBuffer = submitInfo.commandBuffer.as<VulkanQueueCommandBuffer>();

        auto commandBuffer = queueCommandBuffer->getCurrentCommandBuffer();
        vkSubmitInfo.pCommandBuffers = &commandBuffer;
        vkSubmitInfo.waitSemaphoreCount = 1;
        vkSubmitInfo.pWaitSemaphores = &_presentSemaphores[_currentFrame];
        vkSubmitInfo.signalSemaphoreCount = 1;
        vkSubmitInfo.pSignalSemaphores = &_renderSemaphores[_currentFrame];

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vkSubmitInfo.pWaitDstStageMask = waitStages;

        vk::Result result = _queue.submit(1, &vkSubmitInfo, _renderFences[_currentFrame]);
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eQueueSubmitFailed;
        }

        return std::nullopt;
    }

    auto VulkanQueue::present(QueuePresentInfo& presentInfo) noexcept -> std::optional<Error>
    {
        auto* swapchain = presentInfo.swapchain.as<VulkanSwapchain>();

        vk::PresentInfoKHR presentInfoKHR;
        presentInfoKHR.swapchainCount = 1;

        vk::SwapchainKHR swapchainKHR = swapchain->getSwapchain();
        presentInfoKHR.pSwapchains = &swapchainKHR;

        auto imageIndex = static_cast<uint32_t>(swapchain->getCurrentImage());
        presentInfoKHR.pImageIndices = &imageIndex;
        presentInfoKHR.waitSemaphoreCount = 1;
        presentInfoKHR.pWaitSemaphores = &_renderSemaphores[_currentFrame];

        vk::Result result = _queue.presentKHR(&presentInfoKHR);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            return ErrorCode::eSwapchainNeedsResize;
        }
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eQueuePresentFailed;
        }

        return std::nullopt;
    }

    auto VulkanQueue::currentFrame() const noexcept -> size_t
    {
        return _currentFrame;
    }

    auto VulkanQueue::getFramesInFlight() const noexcept -> size_t
    {
        return _framesInFlight;
    }

    auto VulkanQueue::getCurrentPresentSemaphore() const noexcept -> vk::Semaphore
    {
        return _presentSemaphores[_currentFrame];
    }

    auto VulkanQueue::getQueue() const noexcept -> vk::Queue
    {
        return _queue;
    }

    auto VulkanQueue::getFamilyIndex() const noexcept -> uint32_t
    {
        return _familyIndex;
    }
}
