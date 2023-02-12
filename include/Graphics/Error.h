#pragma once
#include <variant>

namespace exage::Graphics
{
    enum class ErrorCode: uint32_t
    {
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

        eCommandBufferBeginFailed,
    };

    using Error = std::variant<ErrorCode>;
} // namespace exage::Graphics
