#pragma once

#include <optional>

#include "Core/Core.h"
#include "Core/Window.h"
#include "glm/glm.hpp"
#include "utils/classes.h"
#include "Context.h"

namespace exage::Graphics
{
    class Texture;
    class CommandBuffer;

    enum class PresentMode
    {
        eImmediate,
        eDoubleBufferVSync,
        eTripleBufferVSync,
    };

    struct SwapchainCreateInfo
    {
        Window& window;
        PresentMode presentMode = PresentMode::eImmediate;
    };

    class EXAGE_EXPORT Swapchain
    {
    public:
        Swapchain() noexcept = default;
        virtual ~Swapchain() = default;
        EXAGE_DELETE_COPY(Swapchain);
        EXAGE_DEFAULT_MOVE(Swapchain);

        [[nodiscard]] virtual auto getPresentMode() const noexcept -> PresentMode = 0;

        [[nodiscard]] virtual auto resize(glm::uvec2 extent) noexcept -> std::optional<Error> = 0;
        [[nodiscard]] virtual auto acquireNextImage(Queue& queue) noexcept -> std::optional<Error> =
        0;
        [[nodiscard]] virtual auto drawImage(CommandBuffer& commandBuffer,
                                             const std::shared_ptr<Texture>& texture) noexcept -> std::optional<Error> = 0;

        EXAGE_BASE_API(API, Swapchain);
    };
} // namespace exage::Graphics
