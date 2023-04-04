#pragma once

#include "exage/Core/Core.h"
#include "exage/Core/Window.h"
#include "exage/Graphics/Error.h"
#include "exage/utils/classes.h"

namespace exage::Graphics
{
    struct SwapchainCreateInfo;
    struct TextureCreateInfo;
    struct BufferCreateInfo;
    struct ShaderCreateInfo;
    struct PipelineCreateInfo;

    enum class API
    {
        eVulkan,
    };

    struct ContextCreateInfo
    {
        API api = API::eVulkan;
        WindowAPI windowAPI;

        Window* optionalWindow = nullptr;
        size_t maxFramesInFlight = 2;
    };

    class CommandBuffer;
    class FrameBuffer;
    class Queue;
    class Swapchain;
    class Texture;
    class Buffer;
    class Shader;
    class Pipeline;
    class ResourceManager;

    struct HardwareSupport
    {
        bool bindlessTexture = false;
        bool bindlessBuffer = false;

        bool bufferAddress = false;  // as in VK_EXT_buffer_device_address
    };

    class EXAGE_EXPORT Context
    {
      public:
        Context() = default;
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        virtual void waitIdle() const noexcept = 0;

        [[nodiscard]] virtual auto getQueue() noexcept -> Queue& = 0;  // Only one queue per context
        [[nodiscard]] virtual auto getQueue() const noexcept -> const Queue& = 0;

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
        [[nodiscard]] virtual auto createShader(const ShaderCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Shader> = 0;
        [[nodiscard]] virtual auto createPipeline(const PipelineCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Pipeline> = 0;
        [[nodiscard]] virtual auto createResourceManager() noexcept
            -> std::unique_ptr<ResourceManager> = 0;

        [[nodiscard]] virtual auto getHardwareSupport() const noexcept -> HardwareSupport = 0;

        EXAGE_BASE_API(API, Context);
        [[nodiscard]] static auto create(ContextCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Context>, Error>;
    };
}  // namespace exage::Graphics
