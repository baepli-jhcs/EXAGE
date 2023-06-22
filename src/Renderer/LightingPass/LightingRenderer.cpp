#include "exage/Renderer/LightingPass/LightingRenderer.h"

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Commands.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/LightingPass/DirectLightingSystem.h"

namespace exage::Renderer
{

    LightingRenderer::LightingRenderer(const LightingRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _extent(createInfo.extent)
        , _directLightingSystem({createInfo.context})
    {
        auto& context = _context.get();

        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
        frameBufferCreateInfo.extent = _extent;
        frameBufferCreateInfo.colorAttachments.resize(1);
        frameBufferCreateInfo.colorAttachments[0] = {
            Graphics::Format::eRGBA16f,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eSampled
                | Graphics::Texture::UsageFlags::eTransferSrc};

        _frameBuffer = context.createFrameBuffer(frameBufferCreateInfo);
    }

    void LightingRenderer::render(Graphics::CommandBuffer& commandBuffer,
                                  Scene& scene,
                                  const LightingRenderInfo& renderInfo) noexcept
    {
        const auto transitionFunc = [&](const auto& texture)
        {
            commandBuffer.textureBarrier(texture,
                                         Graphics::Texture::Layout::eShaderReadOnly,
                                         Graphics::PipelineStageFlags::eColorAttachmentOutput,
                                         Graphics::PipelineStageFlags::eFragmentShader,
                                         Graphics::AccessFlags::eColorAttachmentWrite,
                                         Graphics::AccessFlags::eShaderRead);
        };

        transitionFunc(renderInfo.position);
        transitionFunc(renderInfo.normal);
        transitionFunc(renderInfo.albedo);
        transitionFunc(renderInfo.metallic);
        transitionFunc(renderInfo.roughness);
        transitionFunc(renderInfo.occlusion);
        transitionFunc(renderInfo.emissive);

        for (const auto& texture : _frameBuffer->getTextures())
        {
            commandBuffer.textureBarrier(texture,
                                         Graphics::Texture::Layout::eColorAttachment,
                                         Graphics::PipelineStageFlags::eTopOfPipe,
                                         Graphics::PipelineStageFlags::eColorAttachmentOutput,
                                         Graphics::Access {},
                                         Graphics::AccessFlags::eColorAttachmentWrite);
        }

        Graphics::ClearColor clearColor {};
        clearColor.clear = true;
        clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};

        std::vector clearValues(1, clearColor);

        commandBuffer.beginRendering(_frameBuffer, clearValues, {});

        DirectLightingSystemRenderInfo directLightingSystemRenderInfo {};
        directLightingSystemRenderInfo.position = renderInfo.position;
        directLightingSystemRenderInfo.normal = renderInfo.normal;
        directLightingSystemRenderInfo.albedo = renderInfo.albedo;
        directLightingSystemRenderInfo.metallic = renderInfo.metallic;
        directLightingSystemRenderInfo.roughness = renderInfo.roughness;
        directLightingSystemRenderInfo.occlusion = renderInfo.occlusion;
        directLightingSystemRenderInfo.emissive = renderInfo.emissive;
        directLightingSystemRenderInfo.extent = _extent;

        _directLightingSystem.render(commandBuffer, scene, directLightingSystemRenderInfo);

        commandBuffer.endRendering();
    }

    void LightingRenderer::resize(glm::uvec2 extent) noexcept
    {
        _extent = extent;

        _frameBuffer->resize(extent);
    }

}  // namespace exage::Renderer
