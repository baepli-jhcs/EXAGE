#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/Graphics/Texture.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/utils/classes.h"

namespace exage::Graphics
{

    class VulkanTexture final : public Texture
    {
      public:
        VulkanTexture(VulkanContext& context, const TextureCreateInfo& createInfo) noexcept;
        ~VulkanTexture() override;

        EXAGE_DELETE_COPY(VulkanTexture);
        EXAGE_DELETE_MOVE(VulkanTexture);

        [[nodiscard]] auto getImage() const noexcept -> vk::Image { return _image; }
        [[nodiscard]] auto getImageView(Aspect aspect) const noexcept -> vk::ImageView;
        [[nodiscard]] auto getDepthStencilImageView() const noexcept -> vk::ImageView
        {
            return _depthStencilImageView;
        }

        EXAGE_VULKAN_DERIVED

      private:
        void cleanup() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vma::Allocation _allocation;
        vk::Image _image;
        vk::ImageView _firstImageView;
        vk::ImageView _secondImageView;

        vk::ImageView _depthStencilImageView;
    };
}  // namespace exage::Graphics
