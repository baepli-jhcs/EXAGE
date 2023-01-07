#pragma once

#include "Core/Core.h"

namespace exage::Graphics
{

    struct PrimaryCommandBufferCreateInfo

    {
        size_t maxFramesInFlight = 2;
    };

    class EXAGE_EXPORT PrimaryCommandBuffer
    {
      public:
        PrimaryCommandBuffer() = default;
        ~PrimaryCommandBuffer() = default;
        EXAGE_DELETE_COPY(PrimaryCommandBuffer);
        EXAGE_DEFAULT_MOVE(PrimaryCommandBuffer);
    };
}  // namespace exage::Graphics
