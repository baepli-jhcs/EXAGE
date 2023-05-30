#pragma once

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{
    struct SceneBufferCreateInfo
    {
        Graphics::Context& context;
        size_t initialSize = 100 * 1024 * 1024;
    };

    class SceneBuffer
    {
      public:
        explicit SceneBuffer(const SceneBufferCreateInfo& createInfo) noexcept;
        ~SceneBuffer() = default;

        EXAGE_DELETE_COPY(SceneBuffer);
        EXAGE_DEFAULT_MOVE(SceneBuffer);

        template<typename T>
        [[nodiscard]] auto uploadData(std::span<const T> data,
                                      Graphics::CommandBuffer& commandBuffer,
                                      Graphics::Access access,
                                      Graphics::PipelineStage pipelineStage) noexcept -> size_t
        {
            Block usedBlock {
                .offset = std::numeric_limits<size_t>::max(),
            };

            for (auto it = _freeBlocks.begin(); it != _freeBlocks.end(); ++it)
            {
                auto& block = *it;

                constexpr auto alignment = alignof(T);
                const auto alignedOffset = (block.offset + alignment - 1) & ~(alignment - 1);

                if (block.size >= data.size() * sizeof(T) + alignedOffset)
                {
                    usedBlock.offset = alignedOffset;

                    // the new block's offset is the end of the data, and its size is the remaining
                    // size
                    block.size -= data.size() * sizeof(T) + alignedOffset;
                    block.offset += data.size() * sizeof(T) + alignedOffset;

                    // If the block is empty, remove it
                    if (block.size == 0)
                    {
                        _freeBlocks.erase(it);
                    }

                    break;
                }
            }

            // If no free block was found, resize the buffer
            if (usedBlock.offset == std::numeric_limits<size_t>::max())
            {
                constexpr auto alignment = alignof(T);
                const auto alignedOffset = (_buffer.size() + alignment - 1) & ~(alignment - 1);

                Block block {};
                block.offset = _buffer.size();
                block.size = alignedOffset - _buffer.size();
                _freeBlocks.push_back(block);

                usedBlock.offset = alignedOffset;
                usedBlock.size = data.size() * sizeof(T);

                _buffer.resize(
                    commandBuffer, alignedOffset + data.size() * sizeof(T), access, pipelineStage);
            }

            if (_buffer.get()->isMapped())
            {
                std::span<const std::byte> byteData = std::as_bytes(data);
                _buffer.get()->write(byteData, usedBlock.offset);
            }
            else
            {
                Graphics::BufferCreateInfo createInfo;
                createInfo.size = data.size() * sizeof(T);
                createInfo.mapMode = Graphics::Buffer::MapMode::eMapped;
                createInfo.cached = false;

                auto stagingBuffer = _context.get().createBuffer(createInfo);
                std::span<const std::byte> byteData = std::as_bytes(data);
                stagingBuffer->write(byteData, 0);

                commandBuffer.copyBuffer(
                    stagingBuffer, _buffer.get(), 0, usedBlock.offset, createInfo.size);

                commandBuffer.bufferBarrier(_buffer.get(),
                                            Graphics::PipelineStageFlags::eTransfer,
                                            pipelineStage,
                                            Graphics::AccessFlags::eTransferWrite,
                                            access);
            }

            _usedBlocks.push_back(usedBlock);

            return usedBlock.offset;
        }

        void freeData(size_t offset, size_t size)
        {
            for (auto it = _usedBlocks.begin(); it != _usedBlocks.end(); ++it)
            {
                auto& block = *it;

                if (block.offset == offset && block.size == size)
                {
                    Block freeBlock {};
                    freeBlock.offset = block.offset;
                    freeBlock.size = block.size;

                    _freeBlocks.push_back(freeBlock);

                    _usedBlocks.erase(it);
                    break;
                }
            }
        }

        [[nodiscard]] auto getBuffer() const noexcept -> std::shared_ptr<Graphics::Buffer>
        {
            return _buffer.get();
        }

      private:
        struct Block
        {
            size_t offset;
            size_t size;
        };

        std::reference_wrapper<Graphics::Context> _context;
        Graphics::ResizableBuffer _buffer;

        std::vector<Block> _freeBlocks;
        std::vector<Block> _usedBlocks;
    };

}  // namespace exage::Renderer
