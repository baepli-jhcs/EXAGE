#pragma once

#include "Core/Core.h"
#include "glm/glm.hpp"
#include "utils/classes.h"

namespace exage::Graphics
{
    enum class PresentMode
    {
        eImmediate,
        eDoubleBufferVSync,
        eTripleBufferVSync,
    };

    struct SwapchainCreateInfo
    {
        PresentMode presentMode = PresentMode::eImmediate;
        size_t maxFramesInFlight = 2;
    };

    class EXAGE_EXPORT Swapchain
    {
      public:
        Swapchain() = default;
        virtual ~Swapchain() = default;
        EXAGE_DELETE_COPY(Swapchain);
        EXAGE_DEFAULT_MOVE(Swapchain);

        virtual auto getPresentMode() const -> PresentMode = 0;

        virtual void resize(glm::uvec2 extent);
    };
}  // namespace exage::Graphics
