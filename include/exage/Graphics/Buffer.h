﻿#pragma once

#include <span>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT Buffer
    {
      public:
        enum class AllocationType : uint32_t
        {
            eHost,
            eHostVisible,
            eDevice,
        };

        enum class MemoryUsageFlags : uint32_t
        {
            eMapped = 1 << 0,
            eCached = 1 << 1,
        };

        using MemoryUsage = Flags<MemoryUsageFlags>;

        virtual ~Buffer() = default;

        EXAGE_DEFAULT_COPY(Buffer);
        EXAGE_DEFAULT_MOVE(Buffer);

        virtual void write(std::span<const std::byte> data, size_t offset) noexcept = 0;
        virtual void read(std::span<std::byte> data, size_t offset) const noexcept = 0;

        [[nodiscard]] auto getSize() const noexcept -> size_t { return _size; }
        [[nodiscard]] auto getAllocationType() const noexcept -> AllocationType
        {
            return _allocationType;
        }
        [[nodiscard]] auto getMemoryUsage() const noexcept -> MemoryUsage { return _memoryUsage; }

        EXAGE_BASE_API(API, Buffer);

      protected:
        size_t _size;
        AllocationType _allocationType;
        MemoryUsage _memoryUsage;

        Buffer(uint64_t size, AllocationType allocationType, MemoryUsage memoryUsage) noexcept
            : _size(size)
            , _allocationType(allocationType)
            , _memoryUsage(memoryUsage)
        {
        }
    };

    struct BufferCreateInfo
    {
        size_t size;
        Buffer::AllocationType allocationType;
        Buffer::MemoryUsage memoryUsage;
    };
}  // namespace exage::Graphics