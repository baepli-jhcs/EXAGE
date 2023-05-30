#pragma once

#include <span>

#include <vcruntime.h>

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Scene/Mesh.h"

namespace exage::Renderer
{
    struct GPUSlotBufferCreateInfo
    {
        Graphics::Context& context;
        uint32_t initialElements = 100000;
    };

    template<typename T>
    class GPUSlotBuffer
    {
      public:
        explicit GPUSlotBuffer(const GPUSlotBufferCreateInfo& createInfo) noexcept
            : _context(createInfo.context)
            , _buffer(Graphics::ResizableBufferCreateInfo {{createInfo.initialElements * sizeof(T),
                                                            Graphics::Buffer::MapMode::eIfOptimal,
                                                            false},
                                                           createInfo.context})
        {
            Block block {};
            block.numElements = createInfo.initialElements;
            block.index = 0;
            block.offset = 0;
            _freeBlocks.push_back(block);
        }

        [[nodiscard]] auto uploadData(std::span<const T> data,
                                      Graphics::CommandBuffer& commandBuffer,
                                      Graphics::Access access,
                                      Graphics::PipelineStage pipelineStage) noexcept -> size_t
        {
            Block usedBlock {
                .index = std::numeric_limits<size_t>::max(),
            };

            for (auto it = _freeBlocks.begin(); it != _freeBlocks.end(); ++it)
            {
                auto& block = *it;

                if (block.numElements >= data.size())
                {
                    usedBlock.index = block.index;
                    usedBlock.numElements = data.size();
                    usedBlock.offset = block.offset;

                    // the new block's offset is the end of the data, and its size is the remaining
                    // size
                    block.index += data.size();
                    block.numElements -= data.size();
                    block.offset += data.size() * sizeof(T);

                    // If the block is empty, remove it
                    if (block.numElements == 0)
                    {
                        _freeBlocks.erase(it);
                    }

                    break;
                }
            }

            // If no free block was found, resize the buffer
            if (usedBlock.index == std::numeric_limits<size_t>::max())
            {
                usedBlock.index = _buffer.size() / sizeof(T);
                usedBlock.numElements = data.size();
                usedBlock.offset = _buffer.size();

                _buffer.resize(_buffer.size() + data.size() * sizeof(T));
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
            }

            _usedBlocks.push_back(usedBlock);

            return usedBlock.offset;
        }

        void freeData(size_t offset, size_t numElements)
        {
            for (auto it = _usedBlocks.begin(); it != _usedBlocks.end(); ++it)
            {
                auto& block = *it;

                if (block.offset == offset && block.numElements == numElements)
                {
                    Block freeBlock {};
                    freeBlock.index = block.index;
                    freeBlock.numElements = block.numElements;
                    freeBlock.offset = block.offset;

                    _freeBlocks.push_back(freeBlock);

                    _usedBlocks.erase(it);
                    break;
                }
            }
        }

      private:
        std::reference_wrapper<Graphics::Context> _context;
        Graphics::ResizableBuffer _buffer;

        struct Block
        {
            size_t numElements;
            size_t index;
            size_t offset;
        };

        std::vector<Block> _freeBlocks;
        std::vector<Block> _usedBlocks;
    };

    using IndexBuffer = GPUSlotBuffer<uint32_t>;
    using MeshVertexBuffer = GPUSlotBuffer<MeshVertex>;
}  // namespace exage::Renderer