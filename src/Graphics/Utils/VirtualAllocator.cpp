#include <algorithm>

#include "exage/Graphics/Utils/VirtualAllocator.h"

namespace exage::Graphics
{
    // TODO: this needs major improvements, but I'm not an expert in memory allocators

    VirtualAllocator::VirtualAllocator(uint64_t size) noexcept
        : _size(size)
    {
        _freeBlocks.emplace_back(Block {0, size});
    }

    auto VirtualAllocator::allocate(uint64_t size, uint64_t alignment) noexcept -> uint64_t
    {
        // If alignment is zero, use default alignment
        if (alignment == 0)
        {
            alignment = 1;
        }

        // Find the best matching free block that can fit the requested size and alignment
        uint64_t bestOffset = std::numeric_limits<uint64_t>::max();
        uint64_t bestSize = std::numeric_limits<uint64_t>::max();
        size_t bestIndex = _freeBlocks.size();

        for (size_t i = 0; i < _freeBlocks.size(); i++)
        {
            const Block& block = _freeBlocks[i];

            // Calculate the offset after alignment
            uint64_t alignedOffset = (block.offset + alignment - 1) / alignment * alignment;

            // Check if the block can fit the size
            if (alignedOffset + size <= block.offset + block.size)
            {
                // Check if the block is better than the current best
                uint64_t blockSize = block.size - (alignedOffset - block.offset);
                if (blockSize < bestSize)
                {
                    bestOffset = alignedOffset;
                    bestSize = blockSize;
                    bestIndex = i;
                }
            }
        }

        // If no suitable block was found, return UINT64_MAX
        if (bestOffset == std::numeric_limits<uint64_t>::max())
        {
            return std::numeric_limits<uint64_t>::max();
        }

        // Remove the best block from the free list
        Block bestBlock = _freeBlocks[bestIndex];
        _freeBlocks.erase(_freeBlocks.begin() + bestIndex);

        // Split the best block into two parts: the allocated part and the remaining part
        Block allocatedBlock = {bestOffset, size};
        Block remainingBlock = {bestBlock.offset, bestBlock.size - bestSize};

        // Add the allocated block to the used list
        _usedBlocks.push_back(allocatedBlock);

        // Add the remaining block to the free list if it is not empty
        if (remainingBlock.size > 0)
        {
            _freeBlocks.push_back(remainingBlock);
        }

        // Return the offset of the allocated block
        return allocatedBlock.offset;
    }

    void VirtualAllocator::free(uint64_t offset, uint64_t size) noexcept
    {
        // Find the block that matches the offset and size in the used list
        size_t usedIndex = _usedBlocks.size();
        for (size_t i = 0; i < _usedBlocks.size(); i++)
        {
            const Block& block = _usedBlocks[i];
            if (block.offset == offset && block.size == size)
            {
                usedIndex = i;
                break;
            }
        }

        // If no matching block was found, return
        if (usedIndex == _usedBlocks.size())
        {
            return;
        }

        // Remove the block from the used list
        Block freedBlock = _usedBlocks[usedIndex];
        _usedBlocks.erase(_usedBlocks.begin() + usedIndex);

        // Add the block to the free list
        _freeBlocks.push_back(freedBlock);

        // Sort the free list by offset
        std::sort(_freeBlocks.begin(),
                  _freeBlocks.end(),
                  [](const Block& a, const Block& b) { return a.offset < b.offset; });

        // Merge the block with adjacent free blocks if possible
        for (size_t i = 0; i < _freeBlocks.size() - 1; i++)
        {
            Block& block = _freeBlocks[i];
            Block& nextBlock = _freeBlocks[i + 1];

            // Check if the blocks are adjacent
            if (block.offset + block.size == nextBlock.offset)
            {
                // Merge the blocks
                block.size += nextBlock.size;

                // Remove the next block from the list
                _freeBlocks.erase(_freeBlocks.begin() + i + 1);

                // Repeat the same index
                i--;
            }
        }
    }

    void VirtualAllocator::resize(uint64_t size) noexcept
    {
        if (size < _size)
        {
            return;
        }

        uint64_t offset = _size;
        // If the last block is free, then we can resize it instead of adding a new block
        for (Block& block : _freeBlocks)
        {
            if (block.offset + block.size == _size)
            {
                block.size = size - _size;
                offset = block.offset;
                break;
            }
        }

        if (offset == _size)
        {
            _freeBlocks.emplace_back(Block {_size, size - _size});
        }

        _size = size;
    }

}  // namespace exage::Graphics