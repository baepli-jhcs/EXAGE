#include "exage/Renderer/GeometryPass/GeometryRenderer.h"

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/Texture.h"

namespace exage::Renderer
{
    GeometryRenderer::GeometryRenderer(const GeometryRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _sceneBuffer(createInfo.sceneBuffer)
        , _assetCache(createInfo.assetCache)
        , _extent(createInfo.extent)
        , _meshSystem({createInfo.context,
                       createInfo.sceneBuffer,
                       createInfo.assetCache,
                       createInfo.anisotropy})
    {
        auto& context = _context.get();

        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
        frameBufferCreateInfo.extent = _extent;
        frameBufferCreateInfo.colorAttachments.resize(7);
        frameBufferCreateInfo.colorAttachments[0] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Position

        frameBufferCreateInfo.colorAttachments[1] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Normal

        frameBufferCreateInfo.colorAttachments[2] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Albedo

        frameBufferCreateInfo.colorAttachments[3] = {
            Graphics::Format::eR16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Metallic

        frameBufferCreateInfo.colorAttachments[4] = {
            Graphics::Format::eR16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Roughness

        frameBufferCreateInfo.colorAttachments[5] = {
            Graphics::Format::eR16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Occlusion

        frameBufferCreateInfo.colorAttachments[6] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled};  // Emissive

        frameBufferCreateInfo.depthAttachment = {
            _context.get().getHardwareSupport().depthFormat,
            Graphics::Texture::UsageFlags::eDepthStencilAttachment};

        _frameBuffer = context.createFrameBuffer(frameBufferCreateInfo);
    }

    void GeometryRenderer::render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept
    {
        auto& context = _context.get();

        for (const auto& texture : _frameBuffer->getTextures())
        {
            commandBuffer.textureBarrier(texture,
                                         Graphics::Texture::Layout::eColorAttachment,
                                         Graphics::PipelineStageFlags::eTopOfPipe,
                                         Graphics::PipelineStageFlags::eColorAttachmentOutput,
                                         Graphics::Access {},
                                         Graphics::AccessFlags::eColorAttachmentWrite);
        }

        commandBuffer.textureBarrier(_frameBuffer->getDepthStencilTexture(),
                                     Graphics::Texture::Layout::eDepthStencilAttachment,
                                     Graphics::PipelineStageFlags::eTopOfPipe,
                                     Graphics::PipelineStageFlags::eEarlyFragmentTests,
                                     Graphics::Access {},
                                     Graphics::AccessFlags::eDepthStencilAttachmentWrite);

        Graphics::ClearColor clearColor {};
        clearColor.clear = true;
        clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};

        std::vector clearValues(7, clearColor);

        Graphics::ClearDepthStencil clearDepthStencil;
        clearDepthStencil.clear = true;
        clearDepthStencil.depth = 1.0f;

        commandBuffer.beginRendering(_frameBuffer, clearValues, clearDepthStencil);

        _meshSystem.render(commandBuffer, scene, _extent);

        commandBuffer.endRendering();
    }

    void GeometryRenderer::resize(glm::uvec2 extent) noexcept
    {
        _extent = extent;

        _frameBuffer->resize(extent);
    }

}  // namespace exage::Renderer
