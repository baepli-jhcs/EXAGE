#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanQueue.h"

namespace exage::Graphics
{
    VulkanPrimaryCommandBuffer::VulkanPrimaryCommandBuffer(VulkanContext& context) noexcept
        : _context(context)
    {
        _commandPools.resize(context.getVulkanQueue().getFramesInFlight());
        _commandBuffers.resize(context.getQueue().getFramesInFlight());

        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = context.getVulkanQueue().getFamilyIndex();
        commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        for (size_t i = 0; i < context.getVulkanQueue().getFramesInFlight(); i++)
        {
            vk::Result result = context.getDevice().createCommandPool(
                &commandPoolCreateInfo,
                nullptr,
                &_commandPools[i]);
            vulkanAssertMemoryOut(result);

            vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
            commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
            commandBufferAllocateInfo.commandPool = _commandPools[i];
            commandBufferAllocateInfo.commandBufferCount = 1;

            result = context.getDevice().allocateCommandBuffers(&commandBufferAllocateInfo,
                                                                &_commandBuffers[i]);
            vulkanAssertMemoryOut(result);
        }
    }

    VulkanPrimaryCommandBuffer::~VulkanPrimaryCommandBuffer()
    {
        for (size_t i = 0; i < _context.get().getVulkanQueue().getFramesInFlight(); i++)
        {
            _context.get().getDevice().destroyCommandPool(_commandPools[i]);
        }
    }

    std::optional<Error> VulkanPrimaryCommandBuffer::beginFrame() noexcept
    {
        VulkanContext& context = _context.get();

        vk::CommandBufferBeginInfo commandBufferBeginInfo;
        commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        size_t currentFrame = context.getVulkanQueue().currentFrame();
        _commandBuffers[currentFrame].reset();

        vk::Result result = _commandBuffers[currentFrame].begin(&commandBufferBeginInfo);
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eCommandBufferBeginFailed;
        }

        return std::nullopt;
    }

    std::optional<Error> VulkanPrimaryCommandBuffer::endFrame() noexcept
    {
        _commandBuffers[_context.get().getVulkanQueue().currentFrame()].end();
        return std::nullopt;
    }

    auto VulkanPrimaryCommandBuffer::getCurrentCommandBuffer() const noexcept ->
    vk::CommandBuffer
    {
        return _commandBuffers[_context.get().getQueue().currentFrame()];
    }

    VulkanTemporaryCommandBuffer::VulkanTemporaryCommandBuffer(VulkanContext& context) noexcept
        : _context(context)
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = context.getVulkanQueue().getFamilyIndex();
        commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        vk::Result result = context.getDevice().createCommandPool(
            &commandPoolCreateInfo,
            nullptr,
            &_commandPool);

        vulkanAssertMemoryOut(result);

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        commandBufferAllocateInfo.commandPool = _commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;

        result =
            context.getDevice().allocateCommandBuffers(&commandBufferAllocateInfo, &_commandBuffer);

        vulkanAssertMemoryOut(result);
    }
} // namespace exage::Graphics
