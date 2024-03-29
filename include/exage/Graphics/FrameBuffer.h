﻿#pragma once

#include <optional>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"

namespace exage::Graphics
{
    struct FrameBufferCreateInfo
    {
        glm::uvec2 extent;

        struct ColorAttachmentInfo
        {
            Format format;
            Texture::Usage usage = Texture::UsageFlags::eColorAttachment;
        };

        struct DepthAttachmentInfo
        {
            Format format;
            Texture::Usage usage = Texture::UsageFlags::eDepthStencilAttachment;
        };

        std::vector<ColorAttachmentInfo> colorAttachments;
        std::optional<DepthAttachmentInfo> depthAttachment;
    };

    class FrameBuffer
    {
      public:
        virtual ~FrameBuffer() = default;

        EXAGE_DEFAULT_COPY(FrameBuffer);
        EXAGE_DEFAULT_MOVE(FrameBuffer);

        [[nodiscard]] virtual auto getTexture(size_t index) const noexcept
            -> std::shared_ptr<Texture> = 0;  // returns nullptr for out of bounds
        [[nodiscard]] virtual auto getDepthStencilTexture() const noexcept
            -> std::shared_ptr<Texture> = 0;  // returns nullptr if no depth stencil texture

        [[nodiscard]] virtual auto getTextures() const noexcept
            -> const std::vector<std::shared_ptr<Texture>>& = 0;

        virtual void resize(glm::uvec2 extent) noexcept = 0;
        virtual void attachColor(std::shared_ptr<Texture> texture) noexcept = 0;
        virtual void attachOrReplaceDepthStencil(std::shared_ptr<Texture> texture) noexcept = 0;

        [[nodiscard]] auto getExtent() const noexcept -> glm::uvec2 { return _extent; }

        EXAGE_BASE_API(API, CommandBuffer);

      protected:
        glm::uvec2 _extent;

        FrameBuffer(glm::uvec2 extent) noexcept
            : _extent(extent)
        {
        }
    };
}  // namespace exage::Graphics
