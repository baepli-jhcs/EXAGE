#pragma once

#include <span>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT Buffer
    {
      public:
        enum class MapMode
        {
            eUnmapped,
            eMapped,
            eIfOptimal,
        };

        virtual ~Buffer() = default;

        EXAGE_DEFAULT_COPY(Buffer);
        EXAGE_DEFAULT_MOVE(Buffer);

        virtual void write(std::span<const std::byte> data, size_t offset) noexcept = 0;
        virtual void read(std::span<std::byte> data, size_t offset) const noexcept = 0;

        [[nodiscard]] auto getSize() const noexcept -> size_t { return _size; }
        [[nodiscard]] auto getMapMode() const noexcept -> MapMode
        {
            return _mapMode;
        }
        [[nodiscard]] auto isCached() const noexcept -> bool { return _cached; }

        [[nodiscard]] auto isMapped() const noexcept -> bool { return _isMapped; }

        EXAGE_BASE_API(API, Buffer);

      protected:
        size_t _size;
        MapMode _mapMode;
        bool _cached = false;

        bool _isMapped = false;

        Buffer(uint64_t size, MapMode mapMode, bool cached) noexcept
            : _size(size)
            , _mapMode(mapMode)
            , _cached(cached)
        {
        }
    };

    struct BufferCreateInfo
    {
        size_t size;
        Buffer::MapMode mapMode;
        bool cached = false;
    };
}  // namespace exage::Graphics
