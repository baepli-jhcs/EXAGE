#pragma once

#include <span>

#include "VirtualAllocator.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"

namespace exage::Graphics
{
    struct SlotBufferCreateInfo
    {
        Context& context;
        uint64_t initialSize = 1048576;  // 1MB
    };

    class SlotBuffer
    {
      public:
        explicit SlotBuffer(const SlotBufferCreateInfo& createInfo) noexcept;
        ~SlotBuffer() noexcept = default;

        [[nodiscard]] auto uploadData(std::span<const std::byte> data,
                                      Graphics::CommandBuffer& commandBuffer,
                                      Graphics::Access access,
                                      Graphics::PipelineStage pipelineStage,
                                      uint64_t alignment = 1) noexcept -> uint64_t;

        void freeData(uint64_t offset, uint64_t size) noexcept;

        [[nodiscard]] auto buffer() const noexcept -> std::shared_ptr<Buffer>;

      private:
        Context& _context;
        ResizableBuffer _buffer;

        VirtualAllocator _allocator;

        std::mutex _mutex;
    };
}  // namespace exage::Graphics