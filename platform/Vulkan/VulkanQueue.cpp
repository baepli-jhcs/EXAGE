#include "VulkanQueue.h"

#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"

namespace exage::Graphics
{
    tl::expected<VulkanQueue, Error> VulkanQueue::create(VulkanContext& context,
                                                         const QueueCreateInfo& createInfo) noexcept
    {
        VulkanQueue queue(context, createInfo);
        std::optional<Error> result = queue.init();
        if (result.has_value())
        {
            return tl::make_unexpected(result.value());
        }
        return queue;
    }

    VulkanQueue::VulkanQueue(VulkanContext& context, const QueueCreateInfo& createInfo) noexcept
        : _context(context)
        , _framesInFlight(createInfo.maxFramesInFlight)
    {
    }

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
                &fenceCreateInfo, nullptr, &_renderFences[i]);

            if (fenceResult != vk::Result::eSuccess)
            {
                return ErrorCode::eFenceCreationFailed;
            }

            vk::Result semaphoreResult = _context.get().getDevice().createSemaphore(
                &semaphoreCreateInfo, nullptr, &_presentSemaphores[i]);

            if (semaphoreResult != vk::Result::eSuccess)
            {
                return ErrorCode::eSemaphoreCreationFailed;
            }

            semaphoreResult = _context.get().getDevice().createSemaphore(
                &semaphoreCreateInfo, nullptr, &_renderSemaphores[i]);

            if (semaphoreResult != vk::Result::eSuccess)
            {
                return ErrorCode::eSemaphoreCreationFailed;
            }
        }

        return std::nullopt;
    }

    VulkanQueue::~VulkanQueue()
    {
        cleanup();
    }

    VulkanQueue::VulkanQueue(VulkanQueue&& old) noexcept
        : _context(old._context)
    {
        _framesInFlight = old._framesInFlight;
        _renderFences = std::move(old._renderFences);
        _presentSemaphores = std::move(old._presentSemaphores);
        _renderSemaphores = std::move(old._renderSemaphores);
    }

    void VulkanQueue::cleanup() noexcept
    {
        if (_presentSemaphores.size() != 0)
        {
            _context.get().waitIdle();
        }
        
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

    auto VulkanQueue::operator=(VulkanQueue&& old) noexcept -> VulkanQueue&
    {
        if (this == &old)
        {
            return *this;
        }

        cleanup();

        _context = old._context;

        _framesInFlight = old._framesInFlight;
        _renderFences = std::move(old._renderFences);
        _presentSemaphores = std::move(old._presentSemaphores);
        _renderSemaphores = std::move(old._renderSemaphores);

        return *this;
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

        auto* queueCommandBuffer = submitInfo.commandBuffer.as<VulkanCommandBuffer>();

        auto commandBuffer = queueCommandBuffer->getCommandBuffer();
        vkSubmitInfo.pCommandBuffers = &commandBuffer;
        vkSubmitInfo.waitSemaphoreCount = 1;
        vkSubmitInfo.pWaitSemaphores = &_presentSemaphores[_currentFrame];
        vkSubmitInfo.signalSemaphoreCount = 1;
        vkSubmitInfo.pSignalSemaphores = &_renderSemaphores[_currentFrame];

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vkSubmitInfo.pWaitDstStageMask = waitStages;

        vk::Result result =
            _context.get().getVulkanQueue().submit(1, &vkSubmitInfo, _renderFences[_currentFrame]);
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

        vk::Result result = _context.get().getVulkanQueue().presentKHR(&presentInfoKHR);

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

    auto VulkanQueue::submitTemporary(std::unique_ptr<CommandBuffer> commandBuffer)
        -> std::optional<Error>
    {
        vk::SubmitInfo vkSubmitInfo;
        vkSubmitInfo.commandBufferCount = 1;

        const auto* queueCommandBuffer = commandBuffer->as<VulkanCommandBuffer>();

        const vk::CommandBuffer vkCommand = queueCommandBuffer->getCommandBuffer();
        vkSubmitInfo.pCommandBuffers = &vkCommand;

        vk::Result result = _context.get().getVulkanQueue().submit(1, &vkSubmitInfo, nullptr);
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eQueueSubmitFailed;
        }

        _context.get().waitIdle();
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

    auto VulkanQueue::getCurrentRenderSemaphore() const noexcept -> vk::Semaphore
    {
        return _renderSemaphores[_currentFrame];
    }

    auto VulkanQueue::getCurrentFence() const noexcept -> vk::Fence
    {
        return _renderFences[_currentFrame];
    }
}  // namespace exage::Graphics
