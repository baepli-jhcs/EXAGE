#include "VulkanFrameBuffer.h"

#include "VulkanTexture.h"

namespace exage::Graphics
{
    auto VulkanFrameBuffer::create(VulkanContext& context, glm::uvec2 extent) noexcept
        -> tl::expected<VulkanFrameBuffer, Error>
    {
        VulkanFrameBuffer frameBuffer {context, extent};
        return frameBuffer;
    }

    VulkanFrameBuffer::VulkanFrameBuffer(VulkanContext& context, glm::uvec2 extent) noexcept
        : FrameBuffer(extent)
        , _context(context)
    {
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

    auto VulkanFrameBuffer::resize(glm::uvec2 extent) noexcept -> std::optional<Error>
    {
        for (size_t i = 0; i < _textures.size(); ++i)
        {
            TextureCreateInfo createInfo = _textures[i]->getTextureCreateInfo();
            createInfo.extent = glm::uvec3 {extent.x, extent.y, 1};

            tl::expected texture = _context.createTexture(createInfo);
            if (!texture.has_value())
            {
                return texture.error();
            }

            _textures[i] = std::move(texture.value());
        }

        if (_depthStencilTexture)
        {
            TextureCreateInfo createInfo = _depthStencilTexture->getTextureCreateInfo();
            createInfo.extent = glm::uvec3 {extent.x, extent.y, 1};

            tl::expected texture = _context.createTexture(createInfo);
            if (!texture.has_value())
            {
                return texture.error();
            }

            _depthStencilTexture = std::move(texture.value());
        }

        _extent = extent;
        return std::nullopt;
    }

    auto VulkanFrameBuffer::attachColor(std::shared_ptr<Texture> texture) noexcept
        -> std::optional<Error>
    {
        ASSUME(texture->getExtent() == glm::uvec3 {_extent.x, _extent.y, 1}, "Mismatched framebuffer/texture extent");
        ASSUME(texture->getType() == Texture::Type::e2D, "Bad texture type");
        ASSUME(texture->getUsage().any(Texture::UsageFlags::eColorAttachment), "Not a color attachment");

        _textures.push_back(texture);
        return std::nullopt;
    }

    auto VulkanFrameBuffer::attachOrReplaceDepthStencil(std::shared_ptr<Texture> texture) noexcept
        -> std::optional<Error>
    {
        ASSUME(texture->getExtent() == glm::uvec3 {_extent.x, _extent.y, 1},
               "Mismatched framebuffer/texture extent");
        ASSUME(texture->getType() == Texture::Type::e2D, "Bad texture type");
        ASSUME(texture->getUsage().any(Texture::UsageFlags::eDepthStencilAttachment),
               "Not a depth stencil attachment");

        _depthStencilTexture = texture;
        return std::nullopt;
    }
}  // namespace exage::Graphics
