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
    struct BufferCreateInfo;

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
    class FrameBuffer;
    class Queue;
    class Swapchain;
    class Texture;
    class Buffer;

    class EXAGE_EXPORT Context
    {
      public:
        Context() = default;
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        virtual void waitIdle() const noexcept = 0;
        
        [[nodiscard]] virtual auto getQueue() const noexcept -> Queue& = 0;

        [[nodiscard]] virtual auto createQueue(const QueueCreateInfo& createInfo) noexcept
            -> std::unique_ptr<Queue> = 0;
        [[nodiscard]] virtual auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
            -> std::unique_ptr<Swapchain> = 0;
        [[nodiscard]] virtual auto createCommandBuffer() noexcept
            -> std::unique_ptr<CommandBuffer> = 0;
        [[nodiscard]] virtual auto createTexture(const TextureCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Texture> = 0;
        [[nodiscard]] virtual auto createFrameBuffer(glm::uvec2 extent) noexcept
            -> std::shared_ptr<FrameBuffer> = 0;
        [[nodiscard]] virtual auto createBuffer(const BufferCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Buffer> = 0;

        EXAGE_BASE_API(API, Context);
        [[nodiscard]] static auto create(ContextCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Context>, Error>;
    };
}  // namespace exage::Graphics
