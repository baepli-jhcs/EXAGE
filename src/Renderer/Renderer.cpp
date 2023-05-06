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
        , _resourceManager(createInfo.resourceManager)
        , _extent(createInfo.extent)
        , _forwardRenderer({createInfo.context, createInfo.resourceManager, createInfo.extent})
        , _cameraBuffer({createInfo.context, sizeof(CameraRenderInfo::Data), false})
        , _transformBuffer({createInfo.context, sizeof(TransformRenderInfo::Data), true})
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

        if (_resourceManager)
        {
            _cameraBufferID =
                std::make_shared<Graphics::RAII::BufferID>(*_resourceManager, _cameraBuffer);
            _transformBufferID =
                std::make_shared<Graphics::RAII::BufferID>(*_resourceManager, _transformBuffer);
        }
    }

    void Renderer::render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept
    {
        auto& context = _context.get();
        auto colorTexture = _frameBuffer->getTexture(0);

        auto camera = getSceneCamera(scene);
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

        // Camera buffer
        if (!cameraRenderInfo.buffer)
        {
            constexpr auto cameraBufferSize = sizeof(CameraRenderInfo::Data);

            Graphics::BufferCreateInfo bufferCreateInfo {};
            // size is size until position
            bufferCreateInfo.size = cameraBufferSize;
            bufferCreateInfo.cached = false;
            bufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

            cameraRenderInfo.buffer = context.createBuffer(bufferCreateInfo);

            if (_resourceManager)
            {
                cameraRenderInfo.bufferID = std::make_shared<Graphics::RAII::BufferID>(
                    *_resourceManager, *cameraRenderInfo.buffer);
            }
        }

        commandBuffer.insertDataDependency(cameraRenderInfo.buffer);
        commandBuffer.insertDataDependency(cameraRenderInfo.bufferID);

        std::span<const std::byte> data(reinterpret_cast<const std::byte*>(&cameraRenderInfo.data),
                                        sizeof(CameraRenderInfo::Data));
        cameraRenderInfo.buffer->write(data, 0);

        // Transform buffers

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
