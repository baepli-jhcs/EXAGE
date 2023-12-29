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

        void write(CommandBuffer& commandBuffer,
                   std::span<const std::byte> data,
                   size_t offset,
                   PipelineStage pipelineStage,
                   Access access) noexcept;

        [[nodiscard]] auto hasStagingBuffer() const noexcept -> bool
        {
            return _deviceBuffer != nullptr;
        }

        [[nodiscard]] auto currentHost() const noexcept -> std::shared_ptr<Buffer>;
        [[nodiscard]] auto deviceBuffer() const noexcept -> std::shared_ptr<Buffer>;

        [[nodiscard]] auto currentBindlessID() const noexcept -> BufferID;
        [[nodiscard]] auto currentBuffer() const noexcept -> std::shared_ptr<Buffer>;

      private:
        Context* _context;
        size_t _size;
        std::array<std::shared_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> _hostBuffers;
        std::shared_ptr<Buffer> _deviceBuffer;
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
                    PipelineStage pipelineStage) noexcept;

        // This resize does not perform any copy. All data is lost.
        void resize(size_t newSize) noexcept;

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

        void write(CommandBuffer& commandBuffer,
                   std::span<const std::byte> data,
                   size_t offset,
                   PipelineStage pipelineStage,
                   Access access) noexcept;

        void resize(size_t newSize) noexcept;

        [[nodiscard]] auto hasStagingBuffer() const noexcept -> bool
        {
            return _deviceBuffer.has_value();
        }

        [[nodiscard]] auto currentHost() const noexcept -> std::shared_ptr<Buffer>;
        [[nodiscard]] auto deviceBuffer() const noexcept -> std::shared_ptr<Buffer>;

        [[nodiscard]] auto currentBindlessID() const noexcept -> BufferID;
        [[nodiscard]] auto currentBuffer() const noexcept -> std::shared_ptr<Buffer>;

      private:
        std::reference_wrapper<Context> _context;
        size_t _size;
        std::array<std::optional<ResizableBuffer>, MAX_FRAMES_IN_FLIGHT> _hostBuffers;
        std::optional<ResizableBuffer> _deviceBuffer;
    };

}  // namespace exage::Graphics
