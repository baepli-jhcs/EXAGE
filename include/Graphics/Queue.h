#pragma once

#include "Swapchain.h"
#include "Core/Core.h"

namespace exage::Graphics
{
    using SwapchainSubmits = std::vector<std::reference_wrapper<Swapchain>>;

    class EXAGE_EXPORT Queue
    {
    public:
        Queue() = default;
        virtual ~Queue() = default;
        EXAGE_DELETE_COPY(Queue);
        EXAGE_DEFAULT_MOVE(Queue);

        [[nodiscard]] virtual auto startNextFrame() noexcept -> std::optional<Error> = 0;
        [[nodiscard]] virtual auto submit(
            SwapchainSubmits& submits) noexcept -> std::optional<Error> = 0;

        [[nodiscard]] virtual auto currentFrame() const noexcept -> size_t = 0;

        EXAGE_BASE_API(API, Queue);
    };
}
