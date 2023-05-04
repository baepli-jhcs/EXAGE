#include "exage/Renderer/Scene/SceneBuffer.h"

#include "exage/Graphics/Context.h"
#include "exage/Graphics/Utils/BufferTypes.h"

namespace exage::Renderer
{

    SceneBuffer::SceneBuffer(const SceneBufferCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _buffer(Graphics::ResizableBufferCreateInfo {createInfo.initialSize,
                                                       Graphics::Buffer::MapMode::eIfOptimal,
                                                       false,
                                                       createInfo.context})
    {
        Block block {};
        block.offset = 0;
        block.size = createInfo.initialSize;
        _freeBlocks.push_back(block);
    }

    auto SceneBuffer::uploadData(Graphics::CommandBuffer& commandBuffer,
                                 std::span<const std::byte> data,
                                 Graphics::Access access,
                                 Graphics::PipelineStage pipelineStage) noexcept -> size_t
    {
        // Process: find a free block that fits the data, if not found, resize the buffer

        // Find a free block that fits the data
        size_t offset = 0;

        for (auto it = _freeBlocks.begin(); it != _freeBlocks.end(); ++it)
        {
            auto& block = *it;
            if (block.size >= data.size())
            {
                offset = block.offset;
                block.offset += data.size();
                block.size -= data.size();

                if (block.size == 0)
                {
                    _freeBlocks.erase(it);
                }

                break;
            }
        }

        // If no free block was found, resize the buffer
        if (offset == 0)
        {
            offset = _buffer.size();
            _buffer.resize(commandBuffer, _buffer.size() + data.size(), access, pipelineStage);
        }

        // Upload the data
        if (_buffer.get()->isMapped())
        {
            _buffer.get()->write(data, offset);
        }
        else
        {
            Graphics::BufferCreateInfo createInfo;
            createInfo.size = data.size();
            createInfo.mapMode = Graphics::Buffer::MapMode::eMapped;
            createInfo.cached = false;

            auto buffer = _context.get().createBuffer(createInfo);
            buffer->write(data, offset);

            commandBuffer.copyBuffer(buffer, _buffer.get(), offset, 0, data.size());
        }

        // Add the block to the used blocks
        Block block {};
        block.offset = offset;
        block.size = data.size();
        _usedBlocks.push_back(block);
    }

    void SceneBuffer::freeData(size_t offset, size_t size) noexcept
    {
        // Process: find the block in the used blocks, remove it, and add it to the free blocks

        // Find the block in the used blocks using stl
        auto it = std::find_if(_usedBlocks.begin(),
                               _usedBlocks.end(),
                               [offset, size](auto& block)
                               { return block.offset == offset && block.size == size; });

        // If the block was not found, return
        if (it == _usedBlocks.end())
        {
            return;
        }

        // Remove the block from the used blocks
        _usedBlocks.erase(it);

        // Add the block to the free blocks
        Block block {};
        block.offset = offset;
        block.size = size;

        _freeBlocks.push_back(block);

        // Sort the free blocks by offset
        std::sort(_freeBlocks.begin(),
                  _freeBlocks.end(),
                  [](auto& a, auto& b) { return a.offset < b.offset; });

        // Merge the free blocks
        for (size_t i = 0; i < _freeBlocks.size() - 1; ++i)
        {
            auto& block = _freeBlocks[i];
            auto& nextBlock = _freeBlocks[i + 1];

            if (block.offset + block.size == nextBlock.offset)
            {
                block.size += nextBlock.size;
                _freeBlocks.erase(_freeBlocks.begin() + i + 1);
                --i;
            }
        }
    }

}  // namespace exage::Renderer