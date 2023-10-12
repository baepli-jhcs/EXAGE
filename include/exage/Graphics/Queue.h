#pragma once

#include "CommandBuffer.h"
#include "Swapchain.h"
#include "exage/Core/Core.h"
#include "exage/Graphics/Fence.h"
#include "tl/expected.hpp"

namespace exage::Graphics
{

    class Queue
    {
      public:
        Queue() noexcept = default;
        virtual ~Queue() = default;

        EXAGE_DELETE_COPY(Queue);
        EXAGE_DEFAULT_MOVE(Queue);

        virtual void startNextFrame() noexcept = 0;
        virtual void submit(CommandBuffer& commandBuffer) noexcept = 0;
        [[nodiscard]] virtual auto present(Swapchain& swapchain) noexcept
            -> tl::expected<void, Error> = 0;

        virtual void submitTemporary(std::unique_ptr<CommandBuffer> commandBuffer) noexcept = 0;

        [[nodiscard]] virtual auto currentFrame() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getFramesInFlight() const noexcept -> uint32_t = 0;

        EXAGE_BASE_API(API, Queue);
    };

    class TransferQueue
    {
      public:
        TransferQueue() noexcept = default;
        virtual ~TransferQueue() = default;

        EXAGE_DELETE_COPY(TransferQueue);
        EXAGE_DEFAULT_MOVE(TransferQueue);

        virtual void submit(CommandBuffer& commandBuffer, Fence* fence) noexcept = 0;

        EXAGE_BASE_API(API, TransferQueue);
    };
}  // namespace exage::Graphics
