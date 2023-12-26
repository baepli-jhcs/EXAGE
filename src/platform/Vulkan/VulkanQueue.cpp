#include "exage/platform/Vulkan/VulkanQueue.h"

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Error.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/platform/Vulkan/VulkanCommandBuffer.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/platform/Vulkan/VulkanFence.h"
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

    void VulkanQueue::submit(CommandBuffer& commandBuffer) noexcept
    {
        vk::SubmitInfo vkSubmitInfo;
        vkSubmitInfo.commandBufferCount = 1;

        auto* queueCommandBuffer = commandBuffer.as<VulkanCommandBuffer>();

        auto vkCommandBuffer = queueCommandBuffer->getCommandBuffer();
        vkSubmitInfo.pCommandBuffers = &vkCommandBuffer;
        vkSubmitInfo.waitSemaphoreCount = 1;
        vkSubmitInfo.pWaitSemaphores = &_presentSemaphores[_currentFrame];
        vkSubmitInfo.signalSemaphoreCount = 1;
        vkSubmitInfo.pSignalSemaphores = &_renderSemaphores[_currentFrame];

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vkSubmitInfo.pWaitDstStageMask = waitStages;

        vk::Result const result = _queue.submit(1, &vkSubmitInfo, _renderFences[_currentFrame]);
        checkVulkan(result);
    }

    auto VulkanQueue::present(Swapchain& swapchain) noexcept -> tl::expected<void, Error>
    {
        auto* vulkanSwapchain = swapchain.as<VulkanSwapchain>();

        vk::PresentInfoKHR presentInfoKHR;
        presentInfoKHR.swapchainCount = 1;

        vk::SwapchainKHR const swapchainKHR = vulkanSwapchain->getSwapchain();
        presentInfoKHR.pSwapchains = &swapchainKHR;

        auto imageIndex = static_cast<uint32_t>(vulkanSwapchain->getCurrentImage());
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

    auto VulkanQueue::currentFrame() const noexcept -> uint32_t
    {
        return _currentFrame;
    }

    auto VulkanQueue::getFramesInFlight() const noexcept -> uint32_t
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

    VulkanTransferQueue::VulkanTransferQueue(
        VulkanContext& context, const VulkanTransferQueueCreateInfo& createInfo) noexcept
        : _context(context)
        , _queue(createInfo.queue)
        , _familyIndex(createInfo.familyIndex)
    {
    }

    VulkanTransferQueue::~VulkanTransferQueue()
    {
        cleanup();
    }

    VulkanTransferQueue::VulkanTransferQueue(VulkanTransferQueue&& old) noexcept
        : _context(old._context)
        , _queue(old._queue)
        , _familyIndex(old._familyIndex)
    {
    }

    void VulkanTransferQueue::cleanup() noexcept {}

    auto VulkanTransferQueue::operator=(VulkanTransferQueue&& old) noexcept -> VulkanTransferQueue&
    {
        if (this == &old)
        {
            return *this;
        }

        cleanup();

        _context = old._context;
        _queue = old._queue;
        _familyIndex = old._familyIndex;

        return *this;
    }

    void VulkanTransferQueue::submit(CommandBuffer& commandBuffer, Fence* fence) noexcept
    {
        vk::Fence vkFence = fence != nullptr ? fence->as<VulkanFence>()->getVulkanFence() : nullptr;

        vk::SubmitInfo vkSubmitInfo;
        vkSubmitInfo.commandBufferCount = 1;

        auto* queueCommandBuffer = commandBuffer.as<VulkanCommandBuffer>();

        auto vkCommandBuffer = queueCommandBuffer->getCommandBuffer();
        vkSubmitInfo.pCommandBuffers = &vkCommandBuffer;

        vk::Result const result = _queue.submit(1, &vkSubmitInfo, vkFence);
        checkVulkan(result);
    }
}  // namespace exage::Graphics
