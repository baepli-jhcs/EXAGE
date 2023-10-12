#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    class Fence
    {
      public:
        enum class State
        {
            eSignaled,
            eUnsignaled
        };

        Fence() noexcept = default;
        virtual ~Fence() = default;

        EXAGE_DELETE_COPY(Fence);
        EXAGE_DEFAULT_MOVE(Fence);

        virtual void wait() noexcept = 0;
        virtual void reset() noexcept = 0;

        [[nodiscard]] virtual auto getState() const noexcept -> State = 0;

        EXAGE_BASE_API(API, Fence);
    };
}  // namespace exage::Graphics