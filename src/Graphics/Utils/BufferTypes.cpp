#include "exage/Graphics/Utils/BufferTypes.h"

#include "exage/Core/Debug.h"
#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Queue.h"

namespace exage::Graphics
{
    DynamicFixedBuffer::DynamicFixedBuffer(const DynamicBufferCreateInfo& createInfo)
        : _context(&createInfo.context)
        , _size(createInfo.size)
    {
        auto allocationFlags = Buffer::MapMode::eIfOptimal;

        BufferCreateInfo testBufferCreateInfo {
            .size = createInfo.size, .mapMode = allocationFlags, .cached = createInfo.cached};
        std::shared_ptr<Buffer> testBuffer = createInfo.context.createBuffer(testBufferCreateInfo);

        size_t const framesInFlight = _context->getQueue().getFramesInFlight();

        if (testBuffer->isMapped())
        {
            _hostBuffers[0] = testBuffer;

            testBufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
            for (size_t i = 1; i < framesInFlight; i++)
            {
                _hostBuffers[i] = createInfo.context.createBuffer(testBufferCreateInfo);
            }
        }
        else
        {
            _deviceBuffer = testBuffer;

            testBufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
            for (size_t i = 0; i < framesInFlight; i++)
            {
                _hostBuffers[i] = createInfo.context.createBuffer(testBufferCreateInfo);
            }
        }
    }

    void DynamicFixedBuffer::write(CommandBuffer& commandBuffer,
                                   std::span<const std::byte> data,
                                   size_t offset,
                                   PipelineStage pipelineStage,
                                   Access access) noexcept
    {
        _hostBuffers[_context->getQueue().currentFrame()].get()->write(data, offset);

        if (_deviceBuffer)
        {
            commandBuffer.copyBuffer(
                _hostBuffers[_context->getQueue().currentFrame()], _deviceBuffer, 0, 0, _size);

            commandBuffer.bufferBarrier(_deviceBuffer,
                                        PipelineStageFlags::eTransfer,
                                        pipelineStage,
                                        AccessFlags::eTransferWrite,
                                        access,
                                        QueueOwnership::eUndefined,
                                        QueueOwnership::eUndefined);
        }
    }

    auto DynamicFixedBuffer::currentHost() const noexcept -> std::shared_ptr<Buffer>
    {
        return _hostBuffers[_context->getQueue().currentFrame()];
    }

    auto DynamicFixedBuffer::deviceBuffer() const noexcept -> std::shared_ptr<Buffer>
    {
        return _deviceBuffer;
    }

    auto DynamicFixedBuffer::currentBindlessID() const noexcept -> BufferID
    {
        if (_deviceBuffer)
        {
            return _deviceBuffer->getBindlessID();
        }

        return _hostBuffers[_context->getQueue().currentFrame()]->getBindlessID();
    }

    auto DynamicFixedBuffer::currentBuffer() const noexcept -> std::shared_ptr<Buffer>
    {
        if (_deviceBuffer)
        {
            return _deviceBuffer;
        }

        return _hostBuffers[_context->getQueue().currentFrame()];
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
            .size = newSize,
            .mapMode = _buffer->isMapped() ? Buffer::MapMode::eMapped : Buffer::MapMode::eUnmapped,
            .cached = _buffer->isCached()};
        std::shared_ptr newBuffer = _context.get().createBuffer(createInfo);

        commandBuffer.copyBuffer(_buffer, newBuffer, 0, 0, _buffer->getSize());
        commandBuffer.bufferBarrier(newBuffer,
                                    PipelineStageFlags::eTransfer,
                                    pipelineStage,
                                    AccessFlags::eTransferWrite,
                                    access,
                                    QueueOwnership::eUndefined,
                                    QueueOwnership::eUndefined);

        _buffer = std::move(newBuffer);
    }

    void ResizableBuffer::resize(size_t newSize) noexcept
    {
        _size = newSize;
        if (newSize <= _buffer->getSize())
        {
            return;
        }

        BufferCreateInfo createInfo {
            .size = newSize,
            .mapMode = _buffer->isMapped() ? Buffer::MapMode::eMapped : Buffer::MapMode::eUnmapped,
            .cached = _buffer->isCached()};
        std::shared_ptr newBuffer = _context.get().createBuffer(createInfo);

        _buffer = std::move(newBuffer);
    }

    /*         BufferCreateInfo testBufferCreateInfo {
            .size = createInfo.size, .mapMode = allocationFlags, .cached = createInfo.cached};
        std::shared_ptr<Buffer> testBuffer = createInfo.context.createBuffer(testBufferCreateInfo);

        size_t const framesInFlight = _queue.get().getFramesInFlight();

        if (testBuffer->isMapped())
        {
            _hostBuffers.resize(framesInFlight);
            _hostBuffers[0] = testBuffer;

            testBufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
            for (size_t i = 1; i < framesInFlight; i++)
            {
                _hostBuffers[i] = createInfo.context.createBuffer(testBufferCreateInfo);
            }
        }
        else
        {
            _deviceBuffer = testBuffer;

            testBufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
            for (size_t i = 0; i < framesInFlight; i++)
            {
                _hostBuffers.emplace_back(createInfo.context.createBuffer(testBufferCreateInfo));
            }
        }*/

    ResizableDynamicBuffer::ResizableDynamicBuffer(const DynamicBufferCreateInfo& createInfo)
        : _context(createInfo.context)
        , _size(createInfo.size)
    {
        auto allocationFlags = Buffer::MapMode::eIfOptimal;

        ResizableBufferCreateInfo testBufferCreateInfo {
            createInfo.size, allocationFlags, createInfo.cached, _context.get()};
        ResizableBuffer testBuffer {testBufferCreateInfo};

        size_t const framesInFlight = _context.get().getQueue().getFramesInFlight();

        if (testBuffer.get()->isMapped())
        {
            _hostBuffers[0] = std::move(testBuffer);

            testBufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
            for (size_t i = 1; i < framesInFlight; i++)
            {
                _hostBuffers[i] = ResizableBuffer {testBufferCreateInfo};
            }
        }
        else
        {
            _deviceBuffer = std::move(testBuffer);

            testBufferCreateInfo.mapMode = Buffer::MapMode::eMapped;
            for (size_t i = 0; i < framesInFlight; i++)
            {
                _hostBuffers[i] = ResizableBuffer {testBufferCreateInfo};
            }
        }
    }

    void ResizableDynamicBuffer::write(CommandBuffer& commandBuffer,
                                       std::span<const std::byte> data,
                                       size_t offset,
                                       PipelineStage pipelineStage,
                                       Access access) noexcept
    {
        debugAssume(offset + data.size() <= _size, "Buffer overflow");

        _hostBuffers[_context.get().getQueue().currentFrame()]->get()->write(data, offset);

        if (_deviceBuffer)
        {
            commandBuffer.copyBuffer(_hostBuffers[_context.get().getQueue().currentFrame()]->get(),
                                     _deviceBuffer->get(),
                                     0,
                                     0,
                                     _size);

            commandBuffer.bufferBarrier(_deviceBuffer->get(),
                                        PipelineStageFlags::eTransfer,
                                        pipelineStage,
                                        AccessFlags::eTransferWrite,
                                        access,
                                        QueueOwnership::eUndefined,
                                        QueueOwnership::eUndefined);
        }
    }

    void ResizableDynamicBuffer::resize(size_t newSize) noexcept
    {
        if (newSize == _size)
        {
            return;
        }

        size_t const framesInFlight = _context.get().getQueue().getFramesInFlight();
        for (size_t i = 0; i < framesInFlight; i++)
        {
            _hostBuffers[i]->resize(newSize);
        }

        if (_deviceBuffer)
        {
            _deviceBuffer->resize(newSize);
        }
    }

    auto ResizableDynamicBuffer::currentHost() const noexcept -> std::shared_ptr<Buffer>
    {
        return _hostBuffers[_context.get().getQueue().currentFrame()]->get();
    }

    auto ResizableDynamicBuffer::deviceBuffer() const noexcept -> std::shared_ptr<Buffer>
    {
        return _deviceBuffer->get();
    }

    auto ResizableDynamicBuffer::currentBindlessID() const noexcept -> BufferID
    {
        if (_deviceBuffer)
        {
            return _deviceBuffer->get()->getBindlessID();
        }

        return _hostBuffers[_context.get().getQueue().currentFrame()]->get()->getBindlessID();
    }

    auto ResizableDynamicBuffer::currentBuffer() const noexcept -> std::shared_ptr<Buffer>
    {
        if (_deviceBuffer)
        {
            return _deviceBuffer->get();
        }

        return _hostBuffers[_context.get().getQueue().currentFrame()]->get();
    }

}  // namespace exage::Graphics
