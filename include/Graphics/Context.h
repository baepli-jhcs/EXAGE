#pragma once

#include "Core/Core.h"
#include "Core/Window.h"
#include "utils/classes.h"
#include "Error.h"

namespace exage::Graphics
{
    struct SwapchainCreateInfo;

    enum class API
    {
        eVulkan,
    };

    struct ContextCreateInfo
    {
        API api = API::eVulkan;
        WindowAPI windowAPI;
        size_t maxFramesInFlight = 2;

        Window* optionalWindow = nullptr;
    };

    class QueueCommandBuffer;
    class Queue;
    class Swapchain;

    class EXAGE_EXPORT Context
    {
    public:
        Context() = default;
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        virtual void waitIdle() const noexcept = 0;

        [[nodiscard]] virtual auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<Swapchain>, Error> = 0;
        [[nodiscard]] virtual auto createPrimaryCommandBuffer() noexcept
        -> tl::expected<std::unique_ptr<QueueCommandBuffer>, Error> = 0;

        [[nodiscard]] virtual auto getQueue() noexcept -> Queue& = 0;
        [[nodiscard]] virtual auto getQueue() const noexcept -> const Queue& = 0;

        EXAGE_BASE_API(API, Context);
        [[nodiscard]] static auto create(ContextCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<Context>, Error>;
    };
} // namespace exage::Graphics
