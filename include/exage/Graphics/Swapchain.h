#pragma once

#include <optional>

#include "Context.h"
#include "exage/Core/Core.h"
#include "exage/Core/Window.h"
#include "glm/glm.hpp"
#include "exage/utils/classes.h"

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

        virtual void resize(glm::uvec2 extent) noexcept = 0;
        [[nodiscard]] virtual auto acquireNextImage() noexcept -> std::optional<Error> = 0;
        virtual void drawImage(CommandBuffer& commandBuffer,
                               const std::shared_ptr<Texture>& texture) noexcept = 0;

        EXAGE_BASE_API(API, Swapchain);
    };
}  // namespace exage::Graphics
