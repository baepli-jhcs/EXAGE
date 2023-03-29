﻿#include "exage/Graphics/Utils/BufferTypes.h"

#include "exage/Graphics/Queue.h"

namespace exage::Graphics
{
    DynamicFixedBuffer::DynamicFixedBuffer(const DynamicFixedBufferCreateInfo& createInfo)
        : _queue(createInfo.context.getQueue())
    {
        Buffer::MemoryUsage memoryUsage = Buffer::MemoryUsageFlags::eMapped;
        if (createInfo.cached)
        {
            memoryUsage |= Buffer::MemoryUsageFlags::eCached;
        }

        BufferCreateInfo const hostBuffer {.size = createInfo.size,
                                     .allocationType = Buffer::AllocationType::eHostVisible,
                                     .memoryUsage = memoryUsage};

        size_t const framesInFlight = _queue.get().getFramesInFlight();
        for (size_t i = 0; i < framesInFlight; i++)
        {
            std::shared_ptr buffer = createInfo.context.createBuffer(hostBuffer);
            _hostBuffers.emplace_back(std::move(buffer));
        }

        if (createInfo.useStagingBuffer)
        {
            BufferCreateInfo const deviceBuffer {.size = createInfo.size,
                                           .allocationType = Buffer::AllocationType::eDevice,
                                           .memoryUsage = {}};
            _deviceBuffer = createInfo.context.createBuffer(deviceBuffer);
        }

        _data = std::vector<std::byte>(createInfo.size);
        _dirty = std::vector<bool>(framesInFlight, false);
    }

    void DynamicFixedBuffer::write(std::span<const std::byte> data, size_t offset) noexcept
    {
        std::memcpy(_data.data() + offset, data.data(), data.size());
        for (auto && i : _dirty)
        {
            i = true;
        }
    }
    void DynamicFixedBuffer::readBack(CommandBuffer& commandBuffer) noexcept 
    {
        if (_deviceBuffer)
        {
            commandBuffer.copyBuffer(_deviceBuffer, _hostBuffers[_queue.get().currentFrame()], 0, 0, _data.size());
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

        if (_deviceBuffer)
        {
            commandBuffer.copyBuffer(
                _hostBuffers[_queue.get().currentFrame()], _deviceBuffer, 0, 0, _data.size());
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
}  // namespace exage::Graphics
