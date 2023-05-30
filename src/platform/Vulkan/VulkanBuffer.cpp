#include "exage/platform/Vulkan/VulkanBuffer.h"

#include "vulkan/vulkan_enums.hpp"

namespace exage::Graphics
{
    VulkanBuffer::VulkanBuffer(VulkanContext& context, const BufferCreateInfo& createInfo) noexcept
        : Buffer(createInfo.size, createInfo.mapMode, createInfo.cached)
        , _context(context)
    {
        const vma::Allocator allocator = _context.get().getAllocator();

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = _size;
        // every buffer usage flag
        bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc
            | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformTexelBuffer
            | vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eUniformBuffer
            | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer
            | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndirectBuffer;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = vma::MemoryUsage::eAuto;
        allocInfo.flags = toVmaAllocationCreateFlags(_mapMode, _cached);

        checkVulkan(
            allocator.createBuffer(&bufferInfo, &allocInfo, &_buffer, &_allocation, nullptr));

        if (_mapMode != MapMode::eUnmapped)
        {
            vk::MemoryPropertyFlags flags;
            allocator.getAllocationMemoryProperties(_allocation, &flags);

            if (flags & vk::MemoryPropertyFlagBits::eHostVisible)
            {
                vma::AllocationInfo const info = allocator.getAllocationInfo(_allocation);

                _isMapped = true;
                _mappedData = info.pMappedData;
            }
        }

        _id = _context.get().getResourceManager().bindBuffer(*this);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        cleanup();
    }

    VulkanBuffer::VulkanBuffer(VulkanBuffer&& old) noexcept
        : Buffer(std::move(old))
        , _context(old._context)
        , _buffer(old._buffer)
        , _allocation(old._allocation)
        , _mappedData(old._mappedData)
    {
        old._id = {};

        old._buffer = nullptr;
        old._allocation = nullptr;
        old._mappedData = nullptr;
    }

    auto VulkanBuffer::operator=(VulkanBuffer&& old) noexcept -> VulkanBuffer&
    {
        if (this == &old)
        {
            return *this;
        }

        Buffer::operator=(std::move(old));

        cleanup();

        _context = old._context;

        _buffer = old._buffer;
        _allocation = old._allocation;
        _mappedData = old._mappedData;

        old._id = {};

        old._buffer = nullptr;
        old._allocation = nullptr;
        old._mappedData = nullptr;

        return *this;
    }

    void VulkanBuffer::write(std::span<const std::byte> data, size_t offset) noexcept
    {
        debugAssume(offset + data.size() <= _size, "Buffer overflow");
        debugAssume(_isMapped, "Buffer is not mapped");

        std::memcpy(static_cast<std::byte*>(_mappedData) + offset, data.data(), data.size());
        _context.get().getAllocator().flushAllocation(_allocation, offset, data.size());
    }

    void VulkanBuffer::read(std::span<std::byte> data, size_t offset) const noexcept
    {
        debugAssume(offset + data.size() <= _size, "Buffer overflow");
        debugAssume(_isMapped, "Buffer is not mapped");

        _context.get().getAllocator().invalidateAllocation(_allocation, offset, data.size());
        std::memcpy(data.data(), static_cast<std::byte*>(_mappedData) + offset, data.size());
    }

    void VulkanBuffer::cleanup() noexcept
    {
        if (_id.valid())
        {
            _context.get().getResourceManager().unbindBuffer(_id);
        }

        if (_buffer)
        {
            _context.get().getDevice().destroyBuffer(_buffer);
        }

        if (_allocation)
        {
            _context.get().getAllocator().freeMemory(_allocation);
        }
    }

}  // namespace exage::Graphics
