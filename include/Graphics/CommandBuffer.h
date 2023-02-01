#pragma once

#include "Core/Core.h"
#include "Graphics/Context.h"

namespace exage::Graphics
{
    struct PrimaryCommandBufferCreateInfo { };

    class EXAGE_EXPORT PrimaryCommandBuffer
    {
    public:
        PrimaryCommandBuffer() noexcept = default;
        virtual ~PrimaryCommandBuffer() = default;
        EXAGE_DELETE_COPY(PrimaryCommandBuffer);
        EXAGE_DEFAULT_MOVE(PrimaryCommandBuffer);

        EXAGE_BASE_API(API, PrimaryCommandBuffer);
    };
} // namespace exage::Graphics
