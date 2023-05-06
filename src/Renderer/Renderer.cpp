#include "exage/Renderer/Renderer.h"

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Transform.h"
namespace exage::Renderer
{
    Renderer::Renderer(const RendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _extent(createInfo.extent)
        , _geometryRenderer({createInfo.context, createInfo.extent})
        , _cameraBuffer({createInfo.context, sizeof(CameraRenderInfo), false})
        , _transformBuffer({createInfo.context, sizeof(TransformRenderInfo), true})
    {
        auto& context = _context.get();

        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
        frameBufferCreateInfo.extent = _extent;
        frameBufferCreateInfo.colorAttachments.resize(1);
        frameBufferCreateInfo.colorAttachments[0] = {
            Graphics::Format::eRGBA8,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eTransferSrc
                | Graphics::Texture::UsageFlags::eSampled};

        _frameBuffer = context.createFrameBuffer(frameBufferCreateInfo);
    }

    void Renderer::render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept
    {
        auto& context = _context.get();
        auto colorTexture = _frameBuffer->getTexture(0);

        auto camera = getSceneCamera(scene);

        if (!scene.isValid(camera))
        {
            return;
        }

        auto& cameraComponent = scene.getComponent<Camera>(camera);
        auto& transform = scene.getComponent<Transform3D>(camera);

        // View and projection matrices
        auto view = glm::inverse(transform.globalMatrix);
        auto projection =
            glm::perspective(cameraComponent.fov,
                             static_cast<float>(_extent.x) / static_cast<float>(_extent.y),
                             cameraComponent.near,
                             cameraComponent.far);

        CameraRenderInfo cameraRenderInfo {};
        cameraRenderInfo.view = view;
        cameraRenderInfo.projection = projection;
        cameraRenderInfo.viewProjection = projection * view;
        cameraRenderInfo.position = transform.globalPosition;

        std::span<const std::byte> data(reinterpret_cast<const std::byte*>(&cameraRenderInfo),
                                        sizeof(CameraRenderInfo));
        _cameraBuffer.write(data, 0);

        // Transform buffers

        _geometryRenderer.render(commandBuffer, scene);

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
