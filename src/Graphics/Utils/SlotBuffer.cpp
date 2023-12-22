#include "exage/Graphics/Utils/SlotBuffer.h"

namespace exage::Graphics
{
    SlotBuffer::SlotBuffer(const SlotBufferCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _buffer(ResizableBufferCreateInfo {
              {createInfo.initialSize, Buffer::MapMode::eIfOptimal, false}, createInfo.context})
        , _allocator(createInfo.initialSize)
    {
    }

    auto SlotBuffer::uploadData(std::span<const std::byte> data,
                                CommandBuffer& commandBuffer,
                                Access access,
                                PipelineStage pipelineStage,
                                uint64_t alignment) noexcept -> uint64_t
    {
        std::scoped_lock const lock {_mutex};

        auto allocate = [&]() noexcept -> uint64_t
        {
            uint64_t allocation = _allocator.allocate(data.size(), alignment);

            if (allocation == std::numeric_limits<uint64_t>::max())
            {
                return std::numeric_limits<uint64_t>::max();
            }

            if (_buffer.get()->isMapped())
            {
                _buffer.get()->write(data, allocation);
            }
            else
            {
                BufferCreateInfo bufferCreateInfo {};
                bufferCreateInfo.size = data.size();
                bufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
                bufferCreateInfo.cached = false;

                auto stagingBuffer = _context.createBuffer(bufferCreateInfo);
                stagingBuffer->write(data, 0);

                commandBuffer.copyBuffer(stagingBuffer, _buffer.get(), 0, allocation, data.size());
                commandBuffer.bufferBarrier(_buffer.get(),
                                            PipelineStageFlags::eTransfer,
                                            pipelineStage,
                                            AccessFlags::eTransferWrite,
                                            access,
                                            QueueOwnership::eUndefined,
                                            QueueOwnership::eUndefined);
            }

            return allocation;
        };

        auto allocation = allocate();
        if (allocation == std::numeric_limits<uint64_t>::max())
        {
            _buffer.resize(std::max(_buffer.size() * 2, _buffer.size() + data.size()));
            _allocator.resize(_buffer.size());
            allocation = allocate();

            debugAssert(allocation != std::numeric_limits<uint64_t>::max(),
                        "Failed to allocate memory for slot buffer");
        }

        return allocation;
    }

    void SlotBuffer::freeData(uint64_t offset, uint64_t size) noexcept
    {
        std::scoped_lock const lock {_mutex};

        _allocator.free(offset, size);
    }

    auto SlotBuffer::buffer() const noexcept -> std::shared_ptr<Buffer>
    {
        return _buffer.get();
    }
};  // namespace exage::Graphics