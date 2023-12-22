#pragma once

#include "exage/Core/Core.h"

namespace exage::Graphics
{
    class VirtualAllocator
    {
      public:
        explicit VirtualAllocator(uint64_t size) noexcept;
        ~VirtualAllocator() noexcept = default;

        EXAGE_DELETE_COPY(VirtualAllocator);
        EXAGE_DELETE_MOVE(VirtualAllocator);

        /* Returns the offset of the allocated memory, or UINT64_MAX if the allocation failed */
        [[nodiscard]] auto allocate(uint64_t size, uint64_t alignment = 0) noexcept -> uint64_t;
        void free(uint64_t offset, uint64_t size) noexcept;

        [[nodiscard]] auto size() const noexcept -> uint64_t { return _size; }
        void resize(uint64_t size) noexcept;

      private:
        uint64_t _size;

        struct Block
        {
            uint64_t offset;
            uint64_t size;
        };

        std::vector<Block> _freeBlocks;
        std::vector<Block> _usedBlocks;
    };
}  // namespace exage::Graphics