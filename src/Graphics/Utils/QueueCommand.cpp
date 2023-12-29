#include "exage/Graphics/Utils/QueueCommand.h"

namespace exage::Graphics
{
    QueueCommandRepo::QueueCommandRepo(QueueCommandRepoCreateInfo& createInfo) noexcept
        : _queue(createInfo.context.getQueue())
    {
        for (uint32_t i = 0; i < _queue.get().getFramesInFlight(); i++)
        {
            std::unique_ptr commandBuffer = createInfo.context.createCommandBuffer();
            _commandBuffers[i] = std::move(commandBuffer);
        }
    }

    auto QueueCommandRepo::current() noexcept -> CommandBuffer&
    {
        return *_commandBuffers[_queue.get().currentFrame()];
    }

    auto QueueCommandRepo::current() const noexcept -> const CommandBuffer&
    {
        return *_commandBuffers[_queue.get().currentFrame()];
    }
}  // namespace exage::Graphics
