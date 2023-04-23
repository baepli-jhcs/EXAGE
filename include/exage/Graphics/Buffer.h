#pragma once

#include <span>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT Buffer
    {
      public:
        enum class AllocationFlagBits : uint32_t
        {
            eMapped = 1 << 0,
            eCached = 1 << 1,
            eMappedIfOptimal = 1 << 2,
        };

        using AllocationFlags = Flags<AllocationFlagBits>;

        virtual ~Buffer() = default;

        EXAGE_DEFAULT_COPY(Buffer);
        EXAGE_DEFAULT_MOVE(Buffer);

        virtual void write(std::span<const std::byte> data, size_t offset) noexcept = 0;
        virtual void read(std::span<std::byte> data, size_t offset) const noexcept = 0;

        [[nodiscard]] auto getSize() const noexcept -> size_t { return _size; }
        [[nodiscard]] auto getAllocationFlags() const noexcept -> AllocationFlags { return _allocationFlags; }
        
        [[nodiscard]] auto isMapped() const noexcept -> bool { return _isMapped; }

        EXAGE_BASE_API(API, Buffer);

      protected:
        size_t _size;
        AllocationFlags _allocationFlags;

        bool _isMapped = false;

        Buffer(uint64_t size, AllocationFlags allocationFlags) noexcept
            : _size(size)
            , _allocationFlags(allocationFlags)
        {
        }
    };

    struct BufferCreateInfo
    {
        size_t size;
        Buffer::AllocationFlags allocationFlags;
    };
}  // namespace exage::Graphics
