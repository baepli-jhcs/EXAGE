#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/Graphics/Texture.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/utils/classes.h"
#include "vulkan/vulkan_enums.hpp"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanSampler final : public Sampler
    {
      public:
        VulkanSampler(VulkanContext& context,
                      const SamplerCreateInfo& createInfo,
                      uint32_t mipLevelCount) noexcept;
        ~VulkanSampler() override;

        VulkanSampler(VulkanSampler&&) noexcept;
        auto operator=(VulkanSampler&&) noexcept -> VulkanSampler&;

        [[nodiscard]] auto getSampler() const noexcept -> vk::Sampler { return _sampler; }

        EXAGE_DELETE_COPY(VulkanSampler);

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;
        vk::Sampler _sampler;
    };

    class EXAGE_EXPORT VulkanTexture final : public Texture
    {
      public:
        VulkanTexture(VulkanContext& context, const TextureCreateInfo& createInfo) noexcept;
        ~VulkanTexture() override;

        EXAGE_DELETE_COPY(VulkanTexture);

        VulkanTexture(VulkanTexture&& old) noexcept;
        auto operator=(VulkanTexture&& old) noexcept -> VulkanTexture&;

        [[nodiscard]] auto getSampler() noexcept -> Sampler& override { return *_sampler; }

        [[nodiscard]] auto getSampler() const noexcept -> const Sampler& override
        {
            return *_sampler;
        }

        [[nodiscard]] auto getImage() const noexcept -> vk::Image { return _image; }
        [[nodiscard]] auto getImageView() const noexcept -> vk::ImageView { return _imageView; }

        [[nodiscard]] auto getDescriptorImageInfo(
            vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) const noexcept
            -> vk::DescriptorImageInfo
        {
            return {_sampler->getSampler(), _imageView, layout};
        }

        [[nodiscard]] auto getVulkanSampler() noexcept -> VulkanSampler& { return *_sampler; }
        [[nodiscard]] auto getVulkanSampler() const noexcept -> const VulkanSampler&
        {
            return *_sampler;
        }

        EXAGE_VULKAN_DERIVED

      private:
        void cleanup() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vma::Allocation _allocation;
        vk::Image _image;
        vk::ImageView _imageView;

        std::optional<VulkanSampler> _sampler;
    };
}  // namespace exage::Graphics
