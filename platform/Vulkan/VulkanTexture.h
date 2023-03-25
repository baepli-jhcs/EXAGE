#pragma once

#include "Core/Core.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Texture.h"
#include "Vulkan/VulkanContext.h"
#include "utils/classes.h"

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

        [[nodiscard]] auto getDescriptorImageInfo() const noexcept -> vk::DescriptorImageInfo
        {
            return vk::DescriptorImageInfo(
                _sampler->getSampler(), _imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
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
