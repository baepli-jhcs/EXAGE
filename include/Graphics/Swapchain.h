#pragma once

#include "utils/classes.h"

namespace exage::Graphics
{
    enum class PresentMode
    {
        eImmediate,
        eDoubleBufferVSync,
        eTripleBufferVSync,
    };

    struct SwapchainInfo;

    class Swapchain
    {
      public:
        enum class PresentMode;
        struct CreateInfo;

        Swapchain() = default;
        virtual ~Swapchain() = default;
        EXAGE_DELETE_COPY(Swapchain);
        EXAGE_DEFAULT_MOVE(Swapchain);

        virtual auto getPresentMode() const -> PresentMode = 0;

        enum class PresentMode
        {
            eImmediate,
            eDoubleBufferVSync,
            eTripleBufferVSync,
        };

        struct CreateInfo
        {
            PresentMode presentMode = PresentMode::eImmediate;
            size_t maxFramesInFlight = 2;
        };
    };
}  // namespace exage::Graphics