#pragma once

#include "Core/Core.h"
#include "Core/Window.h"
#include "Error.h"
#include "utils/classes.h"

namespace exage::Graphics
{
    struct QueueCreateInfo;
    struct SwapchainCreateInfo;
    struct TextureCreateInfo;

    enum class API
    {
        eVulkan,
    };

    struct ContextCreateInfo
    {
        API api = API::eVulkan;
        WindowAPI windowAPI;

        Window* optionalWindow = nullptr;
    };

    class CommandBuffer;
    class Queue;
    class Swapchain;
    class Texture;

    class EXAGE_EXPORT Context
    {
      public:
        Context() = default;
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        virtual void waitIdle() const noexcept = 0;

        [[nodiscard]] virtual auto createQueue(const QueueCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Queue>, Error> = 0;
        [[nodiscard]] virtual auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Swapchain>, Error> = 0;
        [[nodiscard]] virtual auto createCommandBuffer() noexcept
            -> tl::expected<std::unique_ptr<CommandBuffer>, Error> = 0;
        [[nodiscard]] virtual auto createTexture(const TextureCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Texture>, Error> = 0;

        EXAGE_BASE_API(API, Context);
        [[nodiscard]] static auto create(ContextCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Context>, Error>;
    };
}  // namespace exage::Graphics
