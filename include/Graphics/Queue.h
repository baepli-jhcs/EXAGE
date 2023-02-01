#pragma once

#include "CommandBuffer.h"
#include "Swapchain.h"
#include "Core/Core.h"

namespace exage::Graphics
{
    struct QueueSubmitInfo
    {
        PrimaryCommandBuffer& commandBuffer; // Only one command buffer per submit is supported
    };

    struct QueuePresentInfo
    {
        Swapchain& swapchain;
        uint32_t imageIndex;
    };


    class EXAGE_EXPORT Queue
    {
    public:
        Queue() noexcept = default;
        virtual ~Queue() = default;
        EXAGE_DELETE_COPY(Queue);
        EXAGE_DEFAULT_MOVE(Queue);

        [[nodiscard]] virtual auto startNextFrame() noexcept -> std::optional<Error> = 0;
        [[nodiscard]] virtual auto submit(
            QueueSubmitInfo& submitInfo) noexcept -> std::optional<Error> = 0;
        [[nodiscard]] virtual auto present(QueuePresentInfo& presentInfo) noexcept
        -> std::optional<Error> = 0;

        [[nodiscard]] virtual auto currentFrame() const noexcept -> size_t = 0;
        [[nodiscard]] virtual auto getFramesInFlight() const noexcept -> size_t = 0;

        EXAGE_BASE_API(API, Queue);
    };
}
