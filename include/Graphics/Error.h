#pragma once

#include <variant>

#include "Core/Core.h"

enum class vk::Result;

namespace exage::Graphics
{
    enum class ErrorCode : uint32_t
    {
        eSwapchainOutOfDate,

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
        eWrongTextureType,

        eFrameBufferTextureExtentMismatch,
        eFrameBufferTextureType,
        eFrameBufferTextureUsage,
    };

    using Error = std::variant<ErrorCode>;
    
    struct FatalVulkanError
    {
        vk::Result result;
    };

    using FatalError = std::variant<FatalVulkanError>;
}  // namespace exage::Graphics
