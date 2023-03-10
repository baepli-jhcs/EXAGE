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
        static auto create(VulkanContext& context,
                           const SamplerCreateInfo& createInfo,
                           uint32_t mipLevelCount) noexcept -> tl::expected<VulkanSampler, Error>;
        ~VulkanSampler() override;

        VulkanSampler(VulkanSampler&&) noexcept;
        auto operator=(VulkanSampler&&) noexcept -> VulkanSampler&;

        [[nodiscard]] auto getSampler() const noexcept -> vk::Sampler { return _sampler; }

        EXAGE_DELETE_COPY(VulkanSampler);

        EXAGE_VULKAN_DERIVED

    private:
        VulkanSampler(VulkanContext& context, const SamplerCreateInfo& createInfo) noexcept;
        [[nodiscard]] auto init(uint32_t mipLevelCount) noexcept -> std::optional<Error>;

        std::reference_wrapper<VulkanContext> _context;
        vk::Sampler _sampler;
    };

    class EXAGE_EXPORT VulkanTexture final : public Texture
    {
    public:
        [[nodiscard]] static auto create(VulkanContext& context,
                                         const TextureCreateInfo& createInfo) noexcept
        -> tl::expected<VulkanTexture, Error>;
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
                _sampler ->getSampler(), _imageView, toVulkanImageLayout(_layout));
        }

        EXAGE_VULKAN_DERIVED
    private:
        VulkanTexture(VulkanContext& context, const TextureCreateInfo& createInfo) noexcept;
        [[nodiscard]] auto init(const SamplerCreateInfo& samplerInfo) noexcept -> std::optional<Error>;

        std::reference_wrapper<VulkanContext> _context;

        vma::Allocation _allocation;
        vk::Image _image;
        vk::ImageView _imageView;

        std::optional<VulkanSampler> _sampler;
    };
} // namespace exage::Graphics
