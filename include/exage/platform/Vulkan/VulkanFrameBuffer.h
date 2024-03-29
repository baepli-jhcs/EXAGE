﻿#pragma once

#include "exage/Graphics/FrameBuffer.h"
#include "exage/platform/Vulkan/VulkanCommandBuffer.h"

namespace exage::Graphics
{
    class VulkanFrameBuffer final : public FrameBuffer
    {
      public:
        VulkanFrameBuffer(VulkanContext& context, glm::uvec2 extent) noexcept;
        VulkanFrameBuffer(VulkanContext& context, const FrameBufferCreateInfo& createInfo) noexcept;
        ~VulkanFrameBuffer() override = default;

        EXAGE_DELETE_COPY(VulkanFrameBuffer);
        EXAGE_DELETE_MOVE(VulkanFrameBuffer);

        [[nodiscard]] auto getTexture(size_t index) const noexcept
            -> std::shared_ptr<Texture> override;
        [[nodiscard]] auto getDepthStencilTexture() const noexcept
            -> std::shared_ptr<Texture> override;

        [[nodiscard]] auto getTextures() const noexcept
            -> const std::vector<std::shared_ptr<Texture>>& override
        {
            return _textures;
        }

        void resize(glm::uvec2 extent) noexcept override;
        void attachColor(std::shared_ptr<Texture> texture) noexcept override;
        void attachOrReplaceDepthStencil(std::shared_ptr<Texture> texture) noexcept override;

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;
        std::vector<std::shared_ptr<Texture>> _textures;
        std::shared_ptr<Texture> _depthStencilTexture;
    };
}  // namespace exage::Graphics
