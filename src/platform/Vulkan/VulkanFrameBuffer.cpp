#include "exage/platform/Vulkan/VulkanFrameBuffer.h"

#include "exage/platform/Vulkan/VulkanTexture.h"

namespace exage::Graphics
{
    VulkanFrameBuffer::VulkanFrameBuffer(VulkanContext& context, glm::uvec2 extent) noexcept
        : FrameBuffer(extent)
        , _context(context)
    {
    }
    VulkanFrameBuffer::VulkanFrameBuffer(VulkanContext& context,
                                         const FrameBufferCreateInfo& createInfo) noexcept
        : VulkanFrameBuffer(context, createInfo.extent)
    {
        TextureCreateInfo colorCreateInfo {};
        colorCreateInfo.extent = glm::uvec3 {createInfo.extent, 1};
        colorCreateInfo.type = Texture::Type::e2D;
        colorCreateInfo.mipLevels = 1;
        colorCreateInfo.arrayLayers = 1;
        colorCreateInfo.samplerCreateInfo.anisotropy = Sampler::Anisotropy::eDisabled;
        colorCreateInfo.samplerCreateInfo.filter = Sampler::Filter::eNearest;
        colorCreateInfo.samplerCreateInfo.mipmapMode = Sampler::MipmapMode::eLinear;

        for (auto& color : createInfo.colorAttachments)
        {
            colorCreateInfo.format = color.format;
            colorCreateInfo.usage = color.usage;
            auto texture = context.createTexture(colorCreateInfo);
            attachColor(texture);
        }

        if (createInfo.depthAttachment)
        {
            TextureCreateInfo depthCreateInfo {};
            depthCreateInfo.extent = glm::uvec3 {createInfo.extent, 1};
            depthCreateInfo.type = Texture::Type::e2D;
            depthCreateInfo.format = createInfo.depthAttachment->format;
            depthCreateInfo.usage = createInfo.depthAttachment->usage;
            auto texture = context.createTexture(depthCreateInfo);
            attachOrReplaceDepthStencil(texture);
        }
    }
    auto VulkanFrameBuffer::getTexture(size_t index) const noexcept -> std::shared_ptr<Texture>
    {
        if (index >= _textures.size())
        {
            return nullptr;
        }

        return _textures[index];
    }
    auto VulkanFrameBuffer::getDepthStencilTexture() const noexcept -> std::shared_ptr<Texture>
    {
        return _depthStencilTexture;
    }

    void VulkanFrameBuffer::resize(glm::uvec2 extent) noexcept
    {
        for (auto& texture : _textures)
        {
            TextureCreateInfo createInfo = texture->getTextureCreateInfo();
            createInfo.extent = glm::uvec3 {extent.x, extent.y, 1};

            texture = _context.get().createTexture(createInfo);
        }

        if (_depthStencilTexture)
        {
            TextureCreateInfo createInfo = _depthStencilTexture->getTextureCreateInfo();
            createInfo.extent = glm::uvec3 {extent.x, extent.y, 1};

            _depthStencilTexture = _context.get().createTexture(createInfo);
        }

        _extent = extent;
    }

    void VulkanFrameBuffer::attachColor(std::shared_ptr<Texture> texture) noexcept

    {
        debugAssume(texture->getExtent() == glm::uvec3 {_extent.x, _extent.y, 1},
                    "Mismatched framebuffer/texture extent");
        debugAssume(texture->getType() == Texture::Type::e2D, "Bad texture type");
        debugAssume(texture->getUsage().any(Texture::UsageFlags::eColorAttachment),
                    "Not a color attachment");

        _textures.push_back(texture);
    }

    void VulkanFrameBuffer::attachOrReplaceDepthStencil(std::shared_ptr<Texture> texture) noexcept
    {
        debugAssume(texture->getExtent() == glm::uvec3 {_extent.x, _extent.y, 1},
                    "Mismatched framebuffer/texture extent");
        debugAssume(texture->getType() == Texture::Type::e2D, "Bad texture type");
        debugAssume(texture->getUsage().any(Texture::UsageFlags::eDepthStencilAttachment),
                    "Not a depth stencil attachment");

        _depthStencilTexture = texture;
    }
}  // namespace exage::Graphics
