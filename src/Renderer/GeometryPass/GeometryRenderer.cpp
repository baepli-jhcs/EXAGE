#include "exage/Renderer/GeometryPass/GeometryRenderer.h"

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/Texture.h"

namespace exage::Renderer
{
    ForwardRenderer::ForwardRenderer(const ForwardRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _extent(createInfo.extent)
    {
        auto& context = _context.get();

        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
        frameBufferCreateInfo.extent = _extent;
        frameBufferCreateInfo.colorAttachments.resize(7);
        frameBufferCreateInfo.colorAttachments[0] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Position

        frameBufferCreateInfo.colorAttachments[1] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Normal

        frameBufferCreateInfo.colorAttachments[2] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Albedo

        frameBufferCreateInfo.colorAttachments[3] = {
            Graphics::Format::eR16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Metallic

        frameBufferCreateInfo.colorAttachments[4] = {
            Graphics::Format::eR16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Roughness

        frameBufferCreateInfo.colorAttachments[5] = {
            Graphics::Format::eR16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Occlusion

        frameBufferCreateInfo.colorAttachments[6] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment};  // Emissive

        frameBufferCreateInfo.depthAttachment = {
            _context.get().getHardwareSupport().depthFormat,
            Graphics::Texture::UsageFlags::eDepthStencilAttachment
        };

        _frameBuffer = context.createFrameBuffer(frameBufferCreateInfo);
    }
}  // namespace exage::Renderer
