#pragma once

#include "Core/Core.h"
#include <variant>

namespace exage::Graphics
{
    enum class ErrorCode : uint32_t
    {
        eInvalidEnum,
        eInvalidAPI,

        eInvalidWindowAPI,
        eInvalidWindow,
        eSurfaceCreationFailed,

        eInstanceCreationFailed,
        ePhysicalDeviceSelectionFailed,
        eDeviceCreationFailed,
        eAllocatorCreationFailed,

        eSwapchainCreationFailed,
        eSwapchainNeedsResize,
        eSwapchainAcquireNextImageFailed,

        eQueueCreationFailed,
        eQueueSubmitFailed,
        eQueuePresentFailed,

        eFenceCreationFailed,
        eFenceWaitFailed,
        eFenceResetFailed,
        eSemaphoreCreationFailed,
        eSemaphoreWaitFailed,

        eCommandPoolCreationFailed,
        eCommandBufferCreationFailed,
        eCommandBufferBeginFailed,
        eCommandBufferEndFailed,

        eSamplerCreationFailed,
        eTextureCreationFailed,
        eTextureViewCreationFailed,

        eWrongTextureLayout,
        eWrongTextureType
    };

    using Error = std::variant<ErrorCode>;
} // namespace exage::Graphics
