﻿#pragma once

#include <optional>

#include "Context.h"
#include "exage/Core/Core.h"
#include "exage/System/Window.h"
#include "exage/utils/classes.h"
#include "glm/glm.hpp"

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
        System::Window& window;
        PresentMode presentMode = PresentMode::eImmediate;
    };

    class Swapchain
    {
      public:
        Swapchain() noexcept = default;
        virtual ~Swapchain() = default;
        EXAGE_DELETE_COPY(Swapchain);
        EXAGE_DEFAULT_MOVE(Swapchain);

        [[nodiscard]] virtual auto getPresentMode() const noexcept -> PresentMode = 0;

        virtual void resize(glm::uvec2 extent) noexcept = 0;
        [[nodiscard]] virtual auto acquireNextImage() noexcept -> tl::expected<void, Error> = 0;
        virtual void drawImage(CommandBuffer& commandBuffer,
                               const std::shared_ptr<Texture>& texture) noexcept = 0;

        EXAGE_BASE_API(API, Swapchain);
    };
}  // namespace exage::Graphics
