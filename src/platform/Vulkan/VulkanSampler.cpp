#include "exage/platform/Vulkan/VulkanSampler.h"

namespace exage::Graphics
{
    VulkanSampler::VulkanSampler(VulkanContext& context,
                                 const SamplerCreateInfo& createInfo) noexcept
        : Sampler(
            createInfo.anisotropy, createInfo.filter, createInfo.mipmapMode, createInfo.lodBias)
        , _context(context)
    {
        vk::Filter const filter = toVulkanFilter(_filter);
        vk::SamplerMipmapMode const mipmapMode = toVulkanSamplerMipmapMode(_mipmapMode);

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable =
            static_cast<vk::Bool32>(_anisotropy != Anisotropy::eDisabled);

        switch (_anisotropy)
        {
            case Anisotropy::eDisabled:
            case Anisotropy::e1:
                samplerInfo.maxAnisotropy = 1.0F;
                break;
            case Anisotropy::e2:
                samplerInfo.maxAnisotropy = 2.0F;
                break;
            case Anisotropy::e4:
                samplerInfo.maxAnisotropy = 4.0F;
                break;
            case Anisotropy::e8:
                samplerInfo.maxAnisotropy = 8.0F;
                break;
            case Anisotropy::e16:
                samplerInfo.maxAnisotropy = 16.0F;
                break;
        }
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerInfo.unnormalizedCoordinates = 0U;
        samplerInfo.compareEnable = 0U;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.mipLodBias = _lodBias;
        samplerInfo.minLod = 0.0F;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        const vk::Result result =
            _context.get().getDevice().createSampler(&samplerInfo, nullptr, &_sampler);
        checkVulkan(result);

        _id = _context.get().getResourceManager().bindSampler(*this);
    }

    VulkanSampler::~VulkanSampler()
    {
        if (_id.valid())
        {
            _context.get().getResourceManager().unbindSampler(_id);
        }

        if (_sampler)
        {
            _context.get().getDevice().destroySampler(_sampler);
        }
    }
}  // namespace exage::Graphics