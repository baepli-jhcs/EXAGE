#include "exage/Graphics/Texture.h"

namespace exage::Graphics
{
    auto Sampler::getSamplerCreateInfo() const noexcept -> SamplerCreateInfo 
    {
        SamplerCreateInfo createInfo;
        createInfo.anisotropy = _anisotropy;
        createInfo.filter = _filter;
        createInfo.mipmapMode = _mipmapMode;
        createInfo.lodBias = _lodBias;
        return createInfo;
    }

    auto Texture::getTextureCreateInfo() const noexcept -> TextureCreateInfo 
    {
        TextureCreateInfo createInfo
        {
            .extent = _extent, .format = _format, .type = _type, .usage = _usage,
            .arrayLayers = _layerCount, .mipLevels = _mipLevelCount,
            .samplerCreateInfo = getSampler().getSamplerCreateInfo()
        };
        
        return createInfo;
    }
}  // namespace exage::Graphics
