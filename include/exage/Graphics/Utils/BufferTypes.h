#pragma once

#include "exage/Graphics/BindlessResources.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Commands.h"

namespace exage::Graphics
{
    struct DynamicBufferCreateInfo
    {
        Context& context;
        size_t size {};
        bool cached = true;
    };

    class DynamicFixedBuffer
    {
      public:
        explicit DynamicFixedBuffer(const DynamicBufferCreateInfo& createInfo);
        ~DynamicFixedBuffer() = default;

        EXAGE_DELETE_COPY(DynamicFixedBuffer);
        EXAGE_DEFAULT_MOVE(DynamicFixedBuffer);

        void write(std::span<const std::byte> data, size_t offset) noexcept;

        void readBack(CommandBuffer& commandBuffer) noexcept;
        void update(CommandBuffer& commandBuffer,
                    PipelineStage pipelineStage,
                    Access access) noexcept;

        [[nodiscard]] auto hasStagingBuffer() const noexcept -> bool
        {
            return _deviceBuffer != nullptr;
        }

        [[nodiscard]] auto currentHost() const noexcept -> std::shared_ptr<Buffer>;
        [[nodiscard]] auto deviceBuffer() const noexcept -> std::shared_ptr<Buffer>;

        [[nodiscard]] auto currentBindlessID() const noexcept -> BufferID;

      private:
        std::reference_wrapper<Queue> _queue;
        std::vector<std::shared_ptr<Buffer>> _hostBuffers;
        std::shared_ptr<Buffer> _deviceBuffer;

        std::vector<std::byte> _data;
        std::vector<bool> _dirty;
    };

    struct ResizableBufferCreateInfo : public BufferCreateInfo
    {
        Context& context;
    };

    class ResizableBuffer
    {
      public:
        explicit ResizableBuffer(const ResizableBufferCreateInfo& createInfo);
        ~ResizableBuffer() = default;

        EXAGE_DEFAULT_COPY(ResizableBuffer);
        EXAGE_DEFAULT_MOVE(ResizableBuffer);

        void resize(CommandBuffer& commandBuffer,
                    size_t newSize,
                    Access access,
                    PipelineStage pipelineStage,
                    bool copy = true) noexcept;

        [[nodiscard]] auto get() const noexcept -> std::shared_ptr<Buffer> { return _buffer; }
        [[nodiscard]] auto size() const noexcept -> size_t { return _buffer->getSize(); }

      private:
        std::reference_wrapper<Context> _context;
        size_t _size;
        std::shared_ptr<Buffer> _buffer;
    };

    class ResizableDynamicBuffer
    {
      public:
        explicit ResizableDynamicBuffer(const DynamicBufferCreateInfo& createInfo);
        ~ResizableDynamicBuffer() = default;

        EXAGE_DELETE_COPY(ResizableDynamicBuffer);
        EXAGE_DEFAULT_MOVE(ResizableDynamicBuffer);

        void write(std::span<const std::byte> data, size_t offset) noexcept;
        void read(std::span<std::byte> data, size_t offset) const noexcept;

        void update(CommandBuffer& commandBuffer,
                    Access access,
                    PipelineStage pipelineStage) noexcept;

        void resize(size_t newSize) noexcept;

        [[nodiscard]] auto hasStagingBuffer() const noexcept -> bool
        {
            return _deviceBuffer.has_value();
        }

        [[nodiscard]] auto currentHost() const noexcept -> std::shared_ptr<Buffer>;
        [[nodiscard]] auto deviceBuffer() const noexcept -> std::shared_ptr<Buffer>;

      private:
        std::reference_wrapper<Context> _context;
        std::vector<ResizableBuffer> _hostBuffers;
        std::optional<ResizableBuffer> _deviceBuffer;

        std::vector<std::byte> _data;
        std::vector<bool> _dirty;

        bool _shouldWrite = false;
    };

}  // namespace exage::Graphics
