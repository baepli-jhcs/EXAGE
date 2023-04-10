#include "exage/Renderer/Renderer.h"

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/Texture.h"

namespace exage::Renderer
{
    Renderer::Renderer(const RendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _extent(createInfo.extent)
        , _forwardRenderer({createInfo.context, createInfo.extent})
    {
        auto& context = _context.get();

        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
        frameBufferCreateInfo.extent = _extent;
        frameBufferCreateInfo.colorAttachments.resize(1);
        frameBufferCreateInfo.colorAttachments[0] = {
            Graphics::Texture::Format::eRGBA8,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eTransferSource};

        _frameBuffer = context.createFrameBuffer(frameBufferCreateInfo);
    }

    void Renderer::render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept
    {
        auto& context = _context.get();
        auto colorTexture = _frameBuffer->getTexture(0);

        commandBuffer.textureBarrier(colorTexture,
                                     Graphics::Texture::Layout::eColorAttachment,
                                     Graphics::PipelineStageFlags::eTopOfPipe,
                                     Graphics::PipelineStageFlags::eColorAttachmentOutput,
                                     Graphics::Access {},
                                     Graphics::AccessFlags::eColorAttachmentWrite);

        Graphics::ClearColor clearColor;
        clearColor.clear = true;
        clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};

        Graphics::ClearDepthStencil clearDepthStencil;
        clearDepthStencil.clear = false;

        commandBuffer.beginRendering(_frameBuffer, {clearColor}, clearDepthStencil);

        commandBuffer.endRendering();
    }

    void Renderer::resize(glm::uvec2 extent) noexcept
    {
        _frameBuffer->resize(extent);
    }
}  // namespace exage::Renderer
