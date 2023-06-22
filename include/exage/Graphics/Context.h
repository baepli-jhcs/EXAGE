#pragma once

#include "exage/Core/Core.h"
#include "exage/Core/Window.h"
#include "exage/Graphics/Error.h"
#include "exage/utils/classes.h"
#include "exage/utils/flags.h"

namespace exage::Graphics
{
    struct SwapchainCreateInfo;
    struct SamplerCreateInfo;
    struct TextureCreateInfo;
    struct FrameBufferCreateInfo;
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
        uint32_t maxFramesInFlight = 2;
    };

    class CommandBuffer;
    class FrameBuffer;
    class Queue;
    class Swapchain;
    class Sampler;
    class Texture;
    class Buffer;
    class Shader;
    class Pipeline;
    class ResourceManager;

    enum class Format : uint32_t;

    struct HardwareSupport
    {
        bool bindlessTexture = false;
        bool bindlessBuffer = false;

        bool bufferAddress = false;  // as in VK_EXT_buffer_device_address
        Format depthFormat;
    };

    struct APIProperties
    {
        bool depthZeroToOne = false;
    };

    enum class FormatFeatureFlags : uint32_t
    {
        eStorageImage = 1 << 0,
        eColorAttachment = 1 << 1,
    };

    using FormatFeatures = Flags<FormatFeatureFlags>;
    EXAGE_ENABLE_FLAGS(FormatFeatures)

    class Context
    {
      public:
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        [[nodiscard]] auto getAPIProperties() const noexcept -> const APIProperties&
        {
            return _apiProperties;
        }

        virtual void waitIdle() const noexcept = 0;

        [[nodiscard]] virtual auto getQueue() noexcept -> Queue& = 0;  // Only one queue per context
        [[nodiscard]] virtual auto getQueue() const noexcept -> const Queue& = 0;

        [[nodiscard]] virtual auto createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
            -> std::unique_ptr<Swapchain> = 0;
        [[nodiscard]] virtual auto createCommandBuffer() noexcept
            -> std::unique_ptr<CommandBuffer> = 0;
        [[nodiscard]] virtual auto createSampler(const SamplerCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Sampler> = 0;
        [[nodiscard]] virtual auto createTexture(const TextureCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Texture> = 0;
        [[nodiscard]] virtual auto createFrameBuffer(glm::uvec2 extent) noexcept
            -> std::shared_ptr<FrameBuffer> = 0;
        [[nodiscard]] virtual auto createFrameBuffer(
            const FrameBufferCreateInfo& createInfo) noexcept -> std::shared_ptr<FrameBuffer> = 0;
        [[nodiscard]] virtual auto createBuffer(const BufferCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Buffer> = 0;
        [[nodiscard]] virtual auto createShader(const ShaderCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Shader> = 0;
        [[nodiscard]] virtual auto createPipeline(const PipelineCreateInfo& createInfo) noexcept
            -> std::shared_ptr<Pipeline> = 0;

        [[nodiscard]] virtual auto getHardwareSupport() const noexcept -> HardwareSupport = 0;
        [[nodiscard]] virtual auto getFormatSupport(Format format) const noexcept
            -> std::pair<bool, FormatFeatures> = 0;  // supported, features

        EXAGE_BASE_API(API, Context);
        [[nodiscard]] static auto create(ContextCreateInfo& createInfo) noexcept
            -> tl::expected<std::unique_ptr<Context>, Error>;

      protected:
        explicit Context(APIProperties apiProperties) noexcept
            : _apiProperties(apiProperties)
        {
        }

        APIProperties _apiProperties;
    };
}  // namespace exage::Graphics
