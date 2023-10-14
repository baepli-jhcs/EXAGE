#pragma once

#include "exage/Core/Core.h"
#include "exage/System/Event.h"

namespace exage::System
{
    enum class StandardCursor
    {
        eArrow,
        eIBeam,
        eCrosshair,
        eHand,
        eHResize,
        eVResize,
    };

    class Cursor  // mostly opaque
    {
      public:
        Cursor() noexcept = default;
        virtual ~Cursor() = default;

        EXAGE_DELETE_COPY(Cursor);
        EXAGE_DEFAULT_MOVE(Cursor);

        EXAGE_BASE_API(API, Cursor);

        static auto create(StandardCursor standardCursor) noexcept -> std::unique_ptr<Cursor>;
        static auto create(glm::uvec2 extent, const uint8_t* pixels, glm::uvec2 hotspot) noexcept
            -> std::unique_ptr<Cursor>;
    };

    void setCursor(Cursor* cursor) noexcept;

}  // namespace exage::System