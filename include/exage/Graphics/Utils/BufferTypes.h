#pragma once

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Commands.h"

namespace exage::Graphics
{
    struct DynamicFixedBufferCreateInfo
    {
        Context& context;
        size_t size;
        bool cached = true;
    };

    class EXAGE_EXPORT DynamicFixedBuffer
    {
      public:
        explicit DynamicFixedBuffer(const DynamicFixedBufferCreateInfo& createInfo);
        ~DynamicFixedBuffer() = default;
        
        EXAGE_DELETE_COPY(DynamicFixedBuffer);
        EXAGE_DEFAULT_MOVE(DynamicFixedBuffer);

        void write(std::span<const std::byte> data, size_t offset) noexcept;
        void read(std::span<std::byte> data, size_t offset) const noexcept;
         
        void readBack(CommandBuffer& commandBuffer) noexcept;
        void update(CommandBuffer& commandBuffer) noexcept;

        [[nodiscard]] auto hasStagingBuffer() const noexcept -> bool
        {
            return _deviceBuffer != nullptr;
        }

        [[nodiscard]] auto currentHost() const noexcept -> std::shared_ptr<Buffer>;
        [[nodiscard]] auto deviceBuffer() const noexcept -> std::shared_ptr<Buffer>;

      private:
        std::reference_wrapper<Queue> _queue;
        std::vector<std::shared_ptr<Buffer>> _hostBuffers;
        std::shared_ptr<Buffer> _deviceBuffer;

        std::vector<std::byte> _data;
        std::vector<bool> _dirty;

        bool shouldWrite = false;
    };

    struct ResizableBufferCreateInfo: public BufferCreateInfo
    {
        Context& context;
    };
    

    class EXAGE_EXPORT ResizableBuffer
    {
      public:
        explicit ResizableBuffer(const ResizableBufferCreateInfo& createInfo);
        ~ResizableBuffer() = default;

        EXAGE_DEFAULT_COPY(ResizableBuffer);
        EXAGE_DEFAULT_MOVE(ResizableBuffer);

        void resize(CommandBuffer& commandBuffer,
                    size_t newSize,
                    Access access,
                    PipelineStage pipelineStage) noexcept;

        [[nodiscard]] auto get() const noexcept -> std::shared_ptr<Buffer> { return _buffer; }
        [[nodiscard]] auto size() const noexcept -> size_t { return _buffer->getSize(); }


      private:
        std::reference_wrapper<Context> _context;
        size_t _size;
        std::shared_ptr<Buffer> _buffer;
    };

}  // namespace exage::Graphics
