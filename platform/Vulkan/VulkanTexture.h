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
                           SamplerCreateInfo createInfo,
                           uint32_t mipLevelCount) noexcept -> tl::expected<VulkanSampler, Error>;
        ~VulkanSampler() override;

        EXAGE_DEFAULT_MOVE(VulkanSampler);
        EXAGE_DELETE_COPY(VulkanSampler);

        [[nodiscard]] auto getAnisotropy() const noexcept -> Anisotropy override
        {
            return _anisotropy;
        }

        [[nodiscard]] auto getFilter() const noexcept -> Filter override { return _filter; }

        [[nodiscard]] auto getMipmapMode() const noexcept -> MipmapMode override
        {
            return _mipmapMode;
        }

        [[nodiscard]] auto getLodBias() const noexcept -> float override { return _lodBias; }

        EXAGE_VULKAN_DERIVED
    private:
        VulkanSampler(VulkanContext& context, SamplerCreateInfo createInfo) noexcept;
        [[nodiscard]] auto init(uint32_t mipLevelCount) noexcept -> std::optional<Error>;

        std::reference_wrapper<VulkanContext> _context;

        Anisotropy _anisotropy;
        Filter _filter;
        MipmapMode _mipmapMode;
        float _lodBias;

        vk::Sampler _sampler;
    };

    class EXAGE_EXPORT VulkanTexture final : public Texture
    {
    public:
        [[nodiscard]] static auto create(VulkanContext& context,
                                         TextureCreateInfo createInfo) noexcept
        -> tl::expected<std::unique_ptr<Texture>, Error>;
        ~VulkanTexture() override;

        EXAGE_DEFAULT_MOVE(VulkanTexture);
        EXAGE_DELETE_COPY(VulkanTexture);

        [[nodiscard]] auto getExtent() const noexcept -> TextureExtent override;
        [[nodiscard]] auto getFormat() const noexcept -> Format override;
        [[nodiscard]] auto getType() const noexcept -> Type override;
        [[nodiscard]] auto getLayout() const noexcept -> Layout override;
        [[nodiscard]] auto getUsage() const noexcept -> Usage override;

        [[nodiscard]] auto getLayerCount() const noexcept -> uint32_t override;
        [[nodiscard]] auto getMipLevelCount() const noexcept -> uint32_t override;

        [[nodiscard]] auto getSampler() noexcept -> Sampler& override;
        [[nodiscard]] auto getSampler() const noexcept -> const Sampler& override;

        EXAGE_VULKAN_DERIVED
    private:
        VulkanTexture(VulkanContext& context, TextureCreateInfo createInfo) noexcept;
        [[nodiscard]] auto init() noexcept -> std::optional<Error>;

        std::reference_wrapper<VulkanContext> _context;

        TextureExtent _extent;
        Format _format;
        Type _type;
        Layout _layout = Layout::eUndefined;
        Usage _usage;

        vma::Allocation _allocation;
        vk::Image _image;
        vk::ImageView _imageView;

        std::optional<VulkanSampler> _sampler;
    };
} // namespace exage::Graphics
