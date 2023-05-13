#include "exage/platform/Vulkan/VulkanQueue.h"

#include "exage/Graphics/Error.h"
#include "exage/platform/Vulkan/VulkanCommandBuffer.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/platform/Vulkan/VulkanSwapchain.h"

namespace exage::Graphics
{

    VulkanQueue::VulkanQueue(VulkanContext& context,
                             const VulkanQueueCreateInfo& createInfo) noexcept
        : _context(context)
        , _framesInFlight(createInfo.maxFramesInFlight)
        , _queue(createInfo.queue)
        , _familyIndex(createInfo.familyIndex)
    {
        _renderFences.resize(_framesInFlight);
        _presentSemaphores.resize(_framesInFlight);
        _renderSemaphores.resize(_framesInFlight);

        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        vk::SemaphoreCreateInfo const semaphoreCreateInfo;

        for (size_t i = 0; i < _framesInFlight; i++)
        {
            vk::Result const fenceResult = _context.get().getDevice().createFence(
                &fenceCreateInfo, nullptr, &_renderFences[i]);

            checkVulkan(fenceResult);

            vk::Result semaphoreResult = _context.get().getDevice().createSemaphore(
                &semaphoreCreateInfo, nullptr, &_presentSemaphores[i]);

            checkVulkan(semaphoreResult);

            semaphoreResult = _context.get().getDevice().createSemaphore(
                &semaphoreCreateInfo, nullptr, &_renderSemaphores[i]);

            checkVulkan(semaphoreResult);
        }
    }

    VulkanQueue::~VulkanQueue()
    {
        cleanup();
    }

    VulkanQueue::VulkanQueue(VulkanQueue&& old) noexcept
        : _context(old._context)
        , _framesInFlight(old._framesInFlight)
        , _queue(old._queue)
        , _familyIndex(old._familyIndex)
        , _renderFences(std::move(old._renderFences))
        , _presentSemaphores(std::move(old._presentSemaphores))
        , _renderSemaphores(std::move(old._renderSemaphores))
    {
    }

    void VulkanQueue::cleanup() noexcept
    {
        if (!_presentSemaphores.empty())
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
        _queue = old._queue;
        _familyIndex = old._familyIndex;
        _renderFences = std::move(old._renderFences);
        _presentSemaphores = std::move(old._presentSemaphores);
        _renderSemaphores = std::move(old._renderSemaphores);

        return *this;
    }

    void VulkanQueue::startNextFrame() noexcept
    {
        _currentFrame = (_currentFrame + 1) % _framesInFlight;

        vk::Result result =
            _context.get().getDevice().waitForFences(1,
                                                     &_renderFences[_currentFrame],
                                                     /*waitAll=*/
                                                     1U,
                                                     std::numeric_limits<uint64_t>::max());
        checkVulkan(result);

        result = _context.get().getDevice().resetFences(1, &_renderFences[_currentFrame]);
        checkVulkan(result);
    }

    void VulkanQueue::submit(QueueSubmitInfo& submitInfo) noexcept
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

        vk::Result const result = _queue.submit(1, &vkSubmitInfo, _renderFences[_currentFrame]);
        checkVulkan(result);
    }

    auto VulkanQueue::present(QueuePresentInfo& presentInfo) noexcept -> tl::expected<void, Error>
    {
        auto* swapchain = presentInfo.swapchain.as<VulkanSwapchain>();

        vk::PresentInfoKHR presentInfoKHR;
        presentInfoKHR.swapchainCount = 1;

        vk::SwapchainKHR const swapchainKHR = swapchain->getSwapchain();
        presentInfoKHR.pSwapchains = &swapchainKHR;

        auto imageIndex = static_cast<uint32_t>(swapchain->getCurrentImage());
        presentInfoKHR.pImageIndices = &imageIndex;
        presentInfoKHR.waitSemaphoreCount = 1;
        presentInfoKHR.pWaitSemaphores = &_renderSemaphores[_currentFrame];

        vk::Result const result = _queue.presentKHR(&presentInfoKHR);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            return tl::make_unexpected(Errors::SwapchainOutOfDate {});
        }

        checkVulkan(result);

        return {};
    }

    void VulkanQueue::submitTemporary(std::unique_ptr<CommandBuffer> commandBuffer) noexcept
    {
        vk::SubmitInfo vkSubmitInfo;
        vkSubmitInfo.commandBufferCount = 1;

        const auto* queueCommandBuffer = commandBuffer->as<VulkanCommandBuffer>();

        const vk::CommandBuffer vkCommand = queueCommandBuffer->getCommandBuffer();
        vkSubmitInfo.pCommandBuffers = &vkCommand;

        vk::Result const result = _queue.submit(1, &vkSubmitInfo, nullptr);
        checkVulkan(result);

        _context.get().waitIdle();
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
