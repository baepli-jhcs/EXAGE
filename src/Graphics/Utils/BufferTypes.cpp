#include "exage/Graphics/Utils/BufferTypes.h"

#include "exage/Graphics/Queue.h"

namespace exage::Graphics
{
    DynamicFixedBuffer::DynamicFixedBuffer(const DynamicFixedBufferCreateInfo& createInfo)
        : _queue(createInfo.context.getQueue())
    {
        auto allocationFlags = Buffer::MapMode::eIfOptimal;

        BufferCreateInfo const hostBuffer {
            .size = createInfo.size, .mapMode = allocationFlags, .cached = createInfo.cached};

        size_t const framesInFlight = _queue.get().getFramesInFlight();
        for (size_t i = 0; i < framesInFlight; i++)
        {
            std::shared_ptr buffer = createInfo.context.createBuffer(hostBuffer);
            _hostBuffers.emplace_back(std::move(buffer));
        }

        if (!_hostBuffers[0]->isMapped())
        {
            BufferCreateInfo const deviceBuffer {.size = createInfo.size,
                                                 .mapMode = Buffer::MapMode::eMapped,
                                                 .cached = createInfo.cached};
            _deviceBuffer = createInfo.context.createBuffer(deviceBuffer);
        }

        _data = std::vector<std::byte>(createInfo.size);
        _dirty = std::vector<bool>(framesInFlight, false);
    }

    void DynamicFixedBuffer::write(std::span<const std::byte> data, size_t offset) noexcept
    {
        std::memcpy(_data.data() + offset, data.data(), data.size());
        for (auto&& i : _dirty)
        {
            i = true;
        }

        shouldWrite = true;
    }
    void DynamicFixedBuffer::readBack(CommandBuffer& commandBuffer) noexcept
    {
        if (_deviceBuffer)
        {
            commandBuffer.copyBuffer(
                _deviceBuffer, _hostBuffers[_queue.get().currentFrame()], 0, 0, _data.size());
        }
    }

    void DynamicFixedBuffer::update(CommandBuffer& commandBuffer) noexcept
    {
        if (!_dirty[_queue.get().currentFrame()])
        {
            return;
        }

        _hostBuffers[_queue.get().currentFrame()]->write(_data, 0);
        _dirty[_queue.get().currentFrame()] = false;

        if (!shouldWrite && _deviceBuffer)
        {
            commandBuffer.copyBuffer(
                _hostBuffers[_queue.get().currentFrame()], _deviceBuffer, 0, 0, _data.size());

            shouldWrite = false;
        }
    }

    auto DynamicFixedBuffer::currentHost() const noexcept -> std::shared_ptr<Buffer>
    {
        return _hostBuffers[_queue.get().currentFrame()];
    }

    auto DynamicFixedBuffer::deviceBuffer() const noexcept -> std::shared_ptr<Buffer>
    {
        return _deviceBuffer;
    }

    ResizableBuffer::ResizableBuffer(const ResizableBufferCreateInfo& createInfo)
        : _context(createInfo.context)
        , _size(createInfo.size)
    {
        _buffer = _context.get().createBuffer(createInfo);
    }

    void ResizableBuffer::resize(CommandBuffer& commandBuffer,
                                 size_t newSize,
                                 Access access,
                                 PipelineStage pipelineStage) noexcept
    {
        _size = newSize;
        if (newSize <= _buffer->getSize())
        {
            return;
        }

        BufferCreateInfo createInfo {
            .size = newSize, .mapMode = _buffer->getMapMode(), .cached = _buffer->isCached()};

        std::shared_ptr newBuffer = _context.get().createBuffer(createInfo);

        commandBuffer.copyBuffer(_buffer, newBuffer, 0, 0, _buffer->getSize());
        commandBuffer.bufferBarrier(newBuffer, PipelineStage {}, pipelineStage, Access {}, access);

        _buffer = std::move(newBuffer);
    }
}  // namespace exage::Graphics
