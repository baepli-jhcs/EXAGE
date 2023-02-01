#include "Vulkan/VulkanCommandBuffer.h"

namespace exage::Graphics
{
    auto VulkanPrimaryCommandBuffer::getCurrentCommandBuffer() const noexcept ->
    vk::CommandBuffer
    {
        return _commandBuffers[_context.get().getQueue().currentFrame()];
    }
} // namespace exage::Graphics
