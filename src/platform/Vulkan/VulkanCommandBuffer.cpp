#include "exage/platform/Vulkan/VulkanCommandBuffer.h"

#include "exage/Graphics/Commands.h"
#include "exage/platform/Vulkan/VulkanBuffer.h"
#include "exage/platform/Vulkan/VulkanFrameBuffer.h"
#include "exage/platform/Vulkan/VulkanPipeline.h"
#include "exage/platform/Vulkan/VulkanQueue.h"
#include "exage/platform/Vulkan/VulkanSampler.h"
#include "exage/platform/Vulkan/VulkanTexture.h"

namespace exage::Graphics
{
    using namespace Commands;

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanContext& context) noexcept
        : _context(context)
    {
        _commandBuffer = _context.get().createVulkanCommandBuffer();

        _commands.reserve(128);
        _dataDependencies.reserve(64);
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        if (_commandBuffer)
        {
            _context.get().destroyCommandBuffer(_commandBuffer);
        }
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& old) noexcept
        : _context(old._context)
        , _commandBuffer(old._commandBuffer)
        , _commands(std::move(old._commands))
        , _dataDependencies(std::move(old._dataDependencies))
        , _currentPipeline(std::move(old._currentPipeline))
    {
        old._commandBuffer = nullptr;
    }

    auto VulkanCommandBuffer::operator=(VulkanCommandBuffer&& old) noexcept -> VulkanCommandBuffer&
    {
        if (this == &old)
        {
            return *this;
        }

        _context = old._context;

        if (_commandBuffer)
        {
            _context.get().destroyCommandBuffer(_commandBuffer);
        }

        _commandBuffer = old._commandBuffer;
        _commands = std::move(old._commands);
        _dataDependencies = std::move(old._dataDependencies);
        _currentPipeline = std::move(old._currentPipeline);

        old._commandBuffer = nullptr;

        return *this;
    }

    void VulkanCommandBuffer::begin() noexcept
    {
        _commandBuffer.reset();
        _commands.clear();
        _dataDependencies.clear();
    }

    void VulkanCommandBuffer::end() noexcept
    {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        vk::Result const result = _commandBuffer.begin(&beginInfo);
        checkVulkan(result);

        for (GPUCommand const& command : _commands)
        {
            processCommand(command);
        }

        checkVulkan(_commandBuffer.end());
    }

    void VulkanCommandBuffer::insertDataDependency(DataDependency dependency) noexcept
    {
        _dataDependencies.push_back(dependency);
    }

    void VulkanCommandBuffer::draw(uint32_t vertexCount,
                                   uint32_t firstVertex,
                                   uint32_t instanceCount,
                                   uint32_t firstInstance) noexcept
    {
        DrawCommand drawCommand {};
        drawCommand.vertexCount = vertexCount;
        drawCommand.instanceCount = instanceCount;
        drawCommand.firstVertex = firstVertex;
        drawCommand.firstInstance = firstInstance;

        _commands.emplace_back(drawCommand);
    }

    void VulkanCommandBuffer::drawIndexed(uint32_t indexCount,
                                          uint32_t firstIndex,
                                          uint32_t vertexOffset,
                                          uint32_t instanceCount,
                                          uint32_t firstInstance) noexcept
    {
        DrawIndexedCommand drawIndexedCommand {};
        drawIndexedCommand.indexCount = indexCount;
        drawIndexedCommand.instanceCount = instanceCount;
        drawIndexedCommand.firstIndex = firstIndex;
        drawIndexedCommand.vertexOffset = vertexOffset;
        drawIndexedCommand.firstInstance = firstInstance;

        _commands.emplace_back(drawIndexedCommand);
    }

    void VulkanCommandBuffer::textureBarrier(std::shared_ptr<Texture> texture,
                                             Texture::Layout newLayout,
                                             PipelineStage srcStage,
                                             PipelineStage dstStage,
                                             Access srcAccess,
                                             Access dstAccess) noexcept
    {
        TextureBarrierCommand textureBarrierCommand;
        textureBarrierCommand.texture = texture;
        textureBarrierCommand.newLayout = newLayout;
        textureBarrierCommand.oldLayout = texture->getLayout();
        textureBarrierCommand.srcStage = srcStage;
        textureBarrierCommand.dstStage = dstStage;
        textureBarrierCommand.srcAccess = srcAccess;
        textureBarrierCommand.dstAccess = dstAccess;

        texture->_layout = newLayout;

        _commands.emplace_back(textureBarrierCommand);
    }

    void VulkanCommandBuffer::bufferBarrier(std::shared_ptr<Buffer> buffer,
                                            PipelineStage srcStage,
                                            PipelineStage dstStage,
                                            Access srcAccess,
                                            Access dstAccess) noexcept
    {
        BufferBarrierCommand bufferBarrierCommand;
        bufferBarrierCommand.buffer = buffer;
        bufferBarrierCommand.srcStage = srcStage;
        bufferBarrierCommand.dstStage = dstStage;
        bufferBarrierCommand.srcAccess = srcAccess;
        bufferBarrierCommand.dstAccess = dstAccess;

        _commands.emplace_back(bufferBarrierCommand);
    }

    void VulkanCommandBuffer::blit(std::shared_ptr<Texture> srcTexture,
                                   std::shared_ptr<Texture> dstTexture,
                                   glm::uvec3 srcOffset,
                                   glm::uvec3 dstOffset,
                                   uint32_t srcMipLevel,
                                   uint32_t dstMipLevel,
                                   uint32_t srcFirstLayer,
                                   uint32_t dstFirstLayer,
                                   uint32_t layerCount,
                                   glm::uvec3 srcExtent,
                                   glm::uvec3 dstExtent) noexcept
    {
        BlitCommand blitCommand;
        blitCommand.srcTexture = srcTexture;
        blitCommand.dstTexture = dstTexture;
        blitCommand.srcOffset = srcOffset;
        blitCommand.dstOffset = dstOffset;
        blitCommand.srcMipLevel = srcMipLevel;
        blitCommand.dstMipLevel = dstMipLevel;
        blitCommand.srcFirstLayer = srcFirstLayer;
        blitCommand.dstFirstLayer = dstFirstLayer;
        blitCommand.layerCount = layerCount;
        blitCommand.srcExtent = srcExtent;
        blitCommand.dstExtent = dstExtent;

        _commands.emplace_back(blitCommand);
    }

    void VulkanCommandBuffer::setViewport(glm::uvec2 offset, glm::uvec2 extent) noexcept
    {
        SetViewportCommand setViewportCommand {};
        setViewportCommand.offset = offset;
        setViewportCommand.extent = extent;

        _commands.emplace_back(setViewportCommand);
    }

    void VulkanCommandBuffer::setScissor(glm::uvec2 offset, glm::uvec2 extent) noexcept
    {
        SetScissorCommand setScissorCommand {};
        setScissorCommand.offset = offset;
        setScissorCommand.extent = extent;

        _commands.emplace_back(setScissorCommand);
    }

    void VulkanCommandBuffer::clearTexture(std::shared_ptr<Texture> texture,
                                           glm::vec4 color,
                                           uint32_t mipLevel,
                                           uint32_t firstLayer,
                                           uint32_t layerCount) noexcept
    {
        debugAssume(texture->getUsage().none(Texture::UsageFlags::eDepthStencilAttachment),
                    "Depth textures cannot be cleared");
        debugAssume(texture->getLayout() == Texture::Layout::eTransferDst,
                    "Image must be in transfer layout");

        ClearTextureCommand clearTextureCommand;
        clearTextureCommand.texture = texture;
        clearTextureCommand.color = color;
        clearTextureCommand.mipLevel = mipLevel;
        clearTextureCommand.firstLayer = firstLayer;
        clearTextureCommand.layerCount = layerCount;

        _commands.emplace_back(clearTextureCommand);
    }

    void VulkanCommandBuffer::beginRendering(std::shared_ptr<FrameBuffer> frameBuffer,
                                             std::vector<ClearColor> clearColors,
                                             ClearDepthStencil clearDepth) noexcept
    {
        debugAssume(frameBuffer->getTextures().size() == clearColors.size(),
                    "There must be one clear color struct set per texture");

        BeginRenderingCommand beginRenderingCommand;
        beginRenderingCommand.frameBuffer = frameBuffer;
        beginRenderingCommand.clearColors = clearColors;
        beginRenderingCommand.clearDepth = clearDepth;

        _commands.emplace_back(beginRenderingCommand);
    }

    void VulkanCommandBuffer::endRendering() noexcept
    {
        EndRenderingCommand endRenderingCommand;

        _commands.emplace_back(endRenderingCommand);
    }

    void VulkanCommandBuffer::copyBuffer(std::shared_ptr<Buffer> srcBuffer,
                                         std::shared_ptr<Buffer> dstBuffer,
                                         uint64_t srcOffset,
                                         uint64_t dstOffset,
                                         uint64_t size) noexcept
    {
        debugAssume(srcBuffer->getSize() >= srcOffset + size,
                    "Source buffer is too small for the copy");
        debugAssume(dstBuffer->getSize() >= dstOffset + size,
                    "Destination buffer is too small for the copy");

        CopyBufferCommand copyBufferCommand;
        copyBufferCommand.srcBuffer = srcBuffer;
        copyBufferCommand.dstBuffer = dstBuffer;
        copyBufferCommand.srcOffset = srcOffset;
        copyBufferCommand.dstOffset = dstOffset;
        copyBufferCommand.size = size;

        _commands.emplace_back(copyBufferCommand);
    }

    void VulkanCommandBuffer::copyBufferToTexture(std::shared_ptr<Buffer> srcBuffer,
                                                  std::shared_ptr<Texture> dstTexture,
                                                  uint64_t srcOffset,
                                                  glm::uvec3 dstOffset,
                                                  uint32_t dstMipLevel,
                                                  uint32_t dstFirstLayer,
                                                  uint32_t layerCount,
                                                  glm::uvec3 extent) noexcept
    {
        debugAssume(dstTexture->getLayout() == Texture::Layout::eTransferDst,
                    "Image must be in transfer layout");

        CopyBufferToTextureCommand copyBufferToTextureCommand;
        copyBufferToTextureCommand.srcBuffer = srcBuffer;
        copyBufferToTextureCommand.dstTexture = dstTexture;
        copyBufferToTextureCommand.srcOffset = srcOffset;
        copyBufferToTextureCommand.dstOffset = dstOffset;
        copyBufferToTextureCommand.dstMipLevel = dstMipLevel;
        copyBufferToTextureCommand.dstFirstLayer = dstFirstLayer;
        copyBufferToTextureCommand.layerCount = layerCount;
        copyBufferToTextureCommand.extent = extent;

        _commands.emplace_back(copyBufferToTextureCommand);
    }

    void VulkanCommandBuffer::copyTextureToBuffer(std::shared_ptr<Texture> srcTexture,
                                                  std::shared_ptr<Buffer> dstBuffer,
                                                  glm::uvec3 srcOffset,
                                                  uint32_t srcMipLevel,
                                                  uint32_t srcFirstLayer,
                                                  uint32_t layerCount,
                                                  glm::uvec3 extent,
                                                  uint64_t dstOffset) noexcept
    {
        debugAssume(srcTexture->getLayout() == Texture::Layout::eTransferSrc,
                    "Image must be in transfer layout");

        CopyTextureToBufferCommand copyTextureToBufferCommand;
        copyTextureToBufferCommand.srcTexture = srcTexture;
        copyTextureToBufferCommand.dstBuffer = dstBuffer;
        copyTextureToBufferCommand.srcOffset = srcOffset;
        copyTextureToBufferCommand.srcMipLevel = srcMipLevel;
        copyTextureToBufferCommand.srcFirstLayer = srcFirstLayer;
        copyTextureToBufferCommand.layerCount = layerCount;
        copyTextureToBufferCommand.extent = extent;
        copyTextureToBufferCommand.dstOffset = dstOffset;

        _commands.emplace_back(copyTextureToBufferCommand);
    }

    void VulkanCommandBuffer::bindPipeline(std::shared_ptr<Pipeline> pipeline) noexcept
    {
        BindPipelineCommand bindPipelineCommand;
        bindPipelineCommand.pipeline = pipeline;

        _commands.emplace_back(bindPipelineCommand);
    }

    void VulkanCommandBuffer::setPushConstant(std::span<const std::byte> data) noexcept
    {
        debugAssume(data.size() <= 128, "Push constant data is too large");

        SetPushConstantCommand setPushConstantCommand {};
        setPushConstantCommand.size = static_cast<uint32_t>(data.size());
        std::memcpy(setPushConstantCommand.data, data.data(), data.size());

        _commands.emplace_back(setPushConstantCommand);
    }

    void VulkanCommandBuffer::bindVertexBuffer(std::shared_ptr<Buffer> buffer,
                                               uint64_t offset) noexcept
    {
        BindVertexBufferCommand bindVertexBufferCommand;
        bindVertexBufferCommand.buffer = buffer;
        bindVertexBufferCommand.offset = offset;

        _commands.emplace_back(bindVertexBufferCommand);
    }

    void VulkanCommandBuffer::bindIndexBuffer(std::shared_ptr<Buffer> buffer,
                                              uint64_t offset) noexcept
    {
        BindIndexBufferCommand bindIndexBufferCommand;
        bindIndexBufferCommand.buffer = buffer;
        bindIndexBufferCommand.offset = offset;

        _commands.emplace_back(bindIndexBufferCommand);
    }

    void VulkanCommandBuffer::bindStorageBuffer(std::shared_ptr<Buffer> buffer,
                                                uint32_t binding) noexcept
    {
        BindStorageBufferCommand bindStorageBufferCommand;
        bindStorageBufferCommand.buffer = buffer;
        bindStorageBufferCommand.binding = binding;

        _commands.emplace_back(bindStorageBufferCommand);
    }

    void VulkanCommandBuffer::bindSampler(std::shared_ptr<Sampler> sampler,
                                          uint32_t binding) noexcept
    {
        BindSamplerCommand bindSamplerCommand;
        bindSamplerCommand.sampler = sampler;
        bindSamplerCommand.binding = binding;

        _commands.emplace_back(bindSamplerCommand);
    }

    void VulkanCommandBuffer::bindSampledTexture(std::shared_ptr<Texture> texture,
                                                 uint32_t binding) noexcept
    {
        BindSampledTextureCommand bindSampledTextureCommand;
        bindSampledTextureCommand.texture = texture;
        bindSampledTextureCommand.binding = binding;

        _commands.emplace_back(bindSampledTextureCommand);
    }

    void VulkanCommandBuffer::bindStorageTexture(std::shared_ptr<Texture> texture,
                                                 uint32_t binding) noexcept
    {
        BindStorageTextureCommand bindStorageTextureCommand;
        bindStorageTextureCommand.texture = texture;
        bindStorageTextureCommand.binding = binding;

        _commands.emplace_back(bindStorageTextureCommand);
    }

    void VulkanCommandBuffer::userDefined(
        std::function<void(CommandBuffer&)> commandFunction) noexcept
    {
        UserDefinedCommand userDefinedCommand;
        userDefinedCommand.commandFunction = commandFunction;

        _commands.emplace_back(userDefinedCommand);
    }

    void VulkanCommandBuffer::processCommand(const GPUCommand& command) noexcept
    {
        std::visit(
            Overload {
                [this](const DrawCommand& draw)
                {
                    _commandBuffer.draw(
                        draw.vertexCount, draw.instanceCount, draw.firstVertex, draw.firstInstance);
                },
                [this](const DrawIndexedCommand& draw)
                {
                    _commandBuffer.drawIndexed(draw.indexCount,
                                               draw.instanceCount,
                                               draw.firstIndex,
                                               draw.vertexOffset,
                                               draw.firstInstance);
                },
                [this](const TextureBarrierCommand& barrier)
                {
                    auto& texture = *barrier.texture->as<VulkanTexture>();

                    vk::ImageMemoryBarrier imageMemoryBarrier;
                    imageMemoryBarrier.oldLayout = toVulkanImageLayout(barrier.oldLayout);
                    imageMemoryBarrier.newLayout = toVulkanImageLayout(barrier.newLayout);
                    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageMemoryBarrier.image = texture.getImage();
                    imageMemoryBarrier.subresourceRange.aspectMask =
                        toVulkanImageAspectFlags(texture.getUsage());
                    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
                    imageMemoryBarrier.subresourceRange.levelCount = texture.getMipLevelCount();
                    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
                    imageMemoryBarrier.subresourceRange.layerCount = texture.getLayerCount();
                    imageMemoryBarrier.srcAccessMask = toVulkanAccessFlags(barrier.srcAccess);
                    imageMemoryBarrier.dstAccessMask = toVulkanAccessFlags(barrier.dstAccess);

                    _commandBuffer.pipelineBarrier(toVulkanPipelineStageFlags(barrier.srcStage),
                                                   toVulkanPipelineStageFlags(barrier.dstStage),
                                                   {},
                                                   0,
                                                   nullptr,
                                                   0,
                                                   nullptr,
                                                   1,
                                                   &imageMemoryBarrier);
                },
                [this](const BufferBarrierCommand& barrier)
                {
                    auto& buffer = *barrier.buffer->as<VulkanBuffer>();

                    vk::BufferMemoryBarrier bufferMemoryBarrier;
                    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferMemoryBarrier.buffer = buffer.getBuffer();
                    bufferMemoryBarrier.offset = 0;
                    bufferMemoryBarrier.size = VK_WHOLE_SIZE;
                    bufferMemoryBarrier.srcAccessMask = toVulkanAccessFlags(barrier.srcAccess);
                    bufferMemoryBarrier.dstAccessMask = toVulkanAccessFlags(barrier.dstAccess);

                    _commandBuffer.pipelineBarrier(toVulkanPipelineStageFlags(barrier.srcStage),
                                                   toVulkanPipelineStageFlags(barrier.dstStage),
                                                   {},
                                                   0,
                                                   nullptr,
                                                   1,
                                                   &bufferMemoryBarrier,
                                                   0,
                                                   nullptr);
                },
                [this](const BlitCommand& copy)
                {
                    auto& srcTexture = *copy.srcTexture->as<VulkanTexture>();
                    auto& dstTexture = *copy.dstTexture->as<VulkanTexture>();

                    vk::ImageBlit imageBlit;
                    imageBlit.srcSubresource.aspectMask =
                        toVulkanImageAspectFlags(srcTexture.getUsage());
                    imageBlit.srcSubresource.mipLevel = copy.srcMipLevel;
                    imageBlit.srcSubresource.baseArrayLayer = copy.srcFirstLayer;
                    imageBlit.srcSubresource.layerCount = copy.layerCount;
                    imageBlit.srcOffsets[0] = vk::Offset3D {static_cast<int32_t>(copy.srcOffset.x),
                                                            static_cast<int32_t>(copy.srcOffset.y),
                                                            static_cast<int32_t>(copy.srcOffset.z)};
                    imageBlit.srcOffsets[1] =
                        vk::Offset3D {static_cast<int32_t>(copy.srcOffset.x + copy.srcExtent.x),
                                      static_cast<int32_t>(copy.srcOffset.y + copy.srcExtent.y),
                                      static_cast<int32_t>(copy.srcOffset.z + copy.srcExtent.z)};
                    imageBlit.dstSubresource.aspectMask =
                        toVulkanImageAspectFlags(dstTexture.getUsage());
                    imageBlit.dstSubresource.mipLevel = copy.dstMipLevel;
                    imageBlit.dstSubresource.baseArrayLayer = copy.dstFirstLayer;
                    imageBlit.dstSubresource.layerCount = copy.layerCount;
                    imageBlit.dstOffsets[0] = vk::Offset3D {static_cast<int32_t>(copy.dstOffset.x),
                                                            static_cast<int32_t>(copy.dstOffset.y),
                                                            static_cast<int32_t>(copy.dstOffset.z)};
                    imageBlit.dstOffsets[1] =
                        vk::Offset3D {static_cast<int32_t>(copy.dstOffset.x + copy.dstExtent.x),
                                      static_cast<int32_t>(copy.dstOffset.y + copy.dstExtent.y),
                                      static_cast<int32_t>(copy.dstOffset.z + copy.dstExtent.z)};

                    _commandBuffer.blitImage(srcTexture.getImage(),
                                             vk::ImageLayout::eTransferSrcOptimal,
                                             dstTexture.getImage(),
                                             vk::ImageLayout::eTransferDstOptimal,
                                             1,
                                             &imageBlit,
                                             vk::Filter::eLinear);
                },
                [this](const UserDefinedCommand& cmd) { cmd.commandFunction(*this); },
                [this](const SetViewportCommand& cmd)
                {
                    // negative height to flip the viewport
                    vk::Viewport viewport {};
                    viewport.x = static_cast<float>(cmd.offset.x);
                    viewport.y =
                        static_cast<float>(cmd.extent.y) - static_cast<float>(cmd.offset.y);
                    viewport.width = static_cast<float>(cmd.extent.x);
                    viewport.height = -static_cast<float>(cmd.extent.y);
                    viewport.maxDepth = 1.0;
                    viewport.minDepth = 0.0;

                    // vk::Viewport viewport {};
                    // viewport.x = static_cast<float>(cmd.offset.x);
                    // viewport.y = static_cast<float>(cmd.offset.y);
                    // viewport.width = static_cast<float>(cmd.extent.x);
                    // viewport.height = static_cast<float>(cmd.extent.y);
                    // viewport.maxDepth = 1.0;
                    // viewport.minDepth = 0.0;

                    _commandBuffer.setViewport(0, viewport);
                },
                [this](const SetScissorCommand& cmd)
                {
                    vk::Rect2D rect {};
                    rect.offset.x = static_cast<int32_t>(cmd.offset.x);
                    rect.offset.y = static_cast<int32_t>(cmd.offset.y);
                    rect.extent.width = cmd.extent.x;
                    rect.extent.height = cmd.extent.y;
                    _commandBuffer.setScissor(0, rect);
                },
                [this](const ClearTextureCommand& cmd)
                {
                    auto& texture = *cmd.texture->as<VulkanTexture>();

                    std::array<float, 4> const color = {
                        cmd.color.x, cmd.color.y, cmd.color.z, cmd.color.w};

                    vk::ClearColorValue value {};
                    value.float32 = color;

                    vk::ImageSubresourceRange subresource {};
                    subresource.baseMipLevel = cmd.mipLevel;
                    subresource.levelCount = 1;
                    subresource.baseArrayLayer = cmd.firstLayer;
                    subresource.layerCount = cmd.layerCount;

                    _commandBuffer.clearColorImage(texture.getImage(),
                                                   vk::ImageLayout::eTransferDstOptimal,
                                                   value,
                                                   subresource);
                },
                [this](const BeginRenderingCommand& cmd)
                {
                    auto& frameBuffer = *cmd.frameBuffer->as<VulkanFrameBuffer>();

                    const std::vector<std::shared_ptr<Texture>>& textures =
                        frameBuffer.getTextures();

                    vk::RenderingInfo renderingInfo {};
                    glm::uvec2 const extent = frameBuffer.getExtent();
                    renderingInfo.renderArea =
                        vk::Rect2D {vk::Offset2D {0, 0}, vk::Extent2D {extent.x, extent.y}};
                    renderingInfo.layerCount = 1;

                    std::vector<vk::RenderingAttachmentInfo> info;
                    info.reserve(textures.size());

                    for (size_t i = 0; i < textures.size(); i++)
                    {
                        auto& vulkanTexture = *textures[i]->as<VulkanTexture>();
                        const auto& clearColor = cmd.clearColors[i];

                        vk::RenderingAttachmentInfo attachmentInfo {};
                        attachmentInfo.imageView = vulkanTexture.getImageView();
                        attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
                        attachmentInfo.resolveMode = vk::ResolveModeFlagBits::eNone;
                        attachmentInfo.loadOp = clearColor.clear ? vk::AttachmentLoadOp::eClear
                                                                 : vk::AttachmentLoadOp::eLoad;
                        attachmentInfo.clearValue.color = std::array<float, 4> {clearColor.color.x,
                                                                                clearColor.color.y,
                                                                                clearColor.color.z,
                                                                                clearColor.color.w};
                        info.push_back(attachmentInfo);
                    }

                    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(info.size());
                    renderingInfo.pColorAttachments = info.data();

                    std::shared_ptr<Texture> const depthTexture =
                        frameBuffer.getDepthStencilTexture();
                    vk::RenderingAttachmentInfo depthAttachmentInfo {};
                    if (depthTexture)
                    {
                        auto& vulkanTexture = *depthTexture->as<VulkanTexture>();
                        const auto& clearDepth = cmd.clearDepth;

                        depthAttachmentInfo.imageView = vulkanTexture.getImageView();
                        depthAttachmentInfo.imageLayout =
                            vk::ImageLayout::eDepthStencilAttachmentOptimal;
                        depthAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eNone;
                        depthAttachmentInfo.loadOp = clearDepth.clear ? vk::AttachmentLoadOp::eClear
                                                                      : vk::AttachmentLoadOp::eLoad;
                        depthAttachmentInfo.clearValue.depthStencil.depth = clearDepth.depth;
                        depthAttachmentInfo.clearValue.depthStencil.stencil = clearDepth.stencil;

                        renderingInfo.pDepthAttachment = &depthAttachmentInfo;
                        renderingInfo.pStencilAttachment = &depthAttachmentInfo;
                    }

                    _commandBuffer.beginRendering(renderingInfo);
                },
                [this](const EndRenderingCommand& /*cmd*/) { _commandBuffer.endRendering(); },
                [this](const CopyBufferCommand& cmd)
                {
                    auto& srcBuffer = *cmd.srcBuffer->as<VulkanBuffer>();
                    auto& dstBuffer = *cmd.dstBuffer->as<VulkanBuffer>();

                    vk::BufferCopy copy {};
                    copy.srcOffset = cmd.srcOffset;
                    copy.dstOffset = cmd.dstOffset;
                    copy.size = cmd.size;

                    _commandBuffer.copyBuffer(srcBuffer.getBuffer(), dstBuffer.getBuffer(), copy);
                },
                [this](const CopyBufferToTextureCommand& cmd)
                {
                    auto& srcBuffer = *cmd.srcBuffer->as<VulkanBuffer>();
                    auto& dstTexture = *cmd.dstTexture->as<VulkanTexture>();

                    vk::BufferImageCopy copy {};
                    copy.bufferOffset = cmd.srcOffset;
                    copy.bufferRowLength = cmd.extent.x;
                    copy.bufferImageHeight = cmd.extent.y;
                    copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    copy.imageSubresource.mipLevel = cmd.dstMipLevel;
                    copy.imageSubresource.baseArrayLayer = cmd.dstFirstLayer;
                    copy.imageSubresource.layerCount = cmd.layerCount;
                    copy.imageOffset.x = cmd.dstOffset.x;
                    copy.imageOffset.y = cmd.dstOffset.y;
                    copy.imageOffset.z = cmd.dstOffset.z;
                    copy.imageExtent.width = cmd.extent.x;
                    copy.imageExtent.height = cmd.extent.y;
                    copy.imageExtent.depth = cmd.extent.z;

                    _commandBuffer.copyBufferToImage(srcBuffer.getBuffer(),
                                                     dstTexture.getImage(),
                                                     vk::ImageLayout::eTransferDstOptimal,
                                                     copy);
                },
                [this](const CopyTextureToBufferCommand& cmd)
                {
                    auto& srcTexture = *cmd.srcTexture->as<VulkanTexture>();
                    auto& dstBuffer = *cmd.dstBuffer->as<VulkanBuffer>();

                    vk::BufferImageCopy copy {};
                    copy.bufferOffset = cmd.dstOffset;
                    copy.bufferRowLength = cmd.extent.x;
                    copy.bufferImageHeight = cmd.extent.y;

                    copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    copy.imageSubresource.mipLevel = cmd.srcMipLevel;
                    copy.imageSubresource.baseArrayLayer = cmd.srcFirstLayer;
                    copy.imageSubresource.layerCount = cmd.layerCount;
                    copy.imageOffset.x = cmd.srcOffset.x;
                    copy.imageOffset.y = cmd.srcOffset.y;
                    copy.imageOffset.z = cmd.srcOffset.z;
                    copy.imageExtent.width = cmd.extent.x;
                    copy.imageExtent.height = cmd.extent.y;
                    copy.imageExtent.depth = cmd.extent.z;

                    _commandBuffer.copyImageToBuffer(srcTexture.getImage(),
                                                     vk::ImageLayout::eTransferSrcOptimal,
                                                     dstBuffer.getBuffer(),
                                                     copy);
                },
                [this](const BindPipelineCommand& cmd)
                {
                    auto& pipeline = *cmd.pipeline->as<VulkanPipeline>();
                    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                                pipeline.getPipeline());

                    if (pipeline.isBindless())
                    {
                        _commandBuffer.bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics,
                            pipeline.getPipelineLayout(),
                            0,
                            _context.get().getResourceManager().getDescriptorSet(),
                            {});
                    }

                    _currentPipeline = &pipeline;
                },
                [this](const SetPushConstantCommand& cmd)
                {
                    _commandBuffer.pushConstants(_currentPipeline->getPipelineLayout(),
                                                 vk::ShaderStageFlagBits::eAll,
                                                 0,
                                                 cmd.size,
                                                 cmd.data);
                },
                [this](const BindVertexBufferCommand& cmd)
                {
                    auto& buffer = *cmd.buffer->as<VulkanBuffer>();
                    vk::DeviceSize offset = 0;
                    _commandBuffer.bindVertexBuffers(0, buffer.getBuffer(), offset);
                },
                [this](const BindIndexBufferCommand& cmd)
                {
                    auto& buffer = *cmd.buffer->as<VulkanBuffer>();
                    _commandBuffer.bindIndexBuffer(buffer.getBuffer(), 0, vk::IndexType::eUint32);
                },
                [this](const BindSampledTextureCommand& cmd)
                {
                    auto& texture = *cmd.texture->as<VulkanTexture>();
                    vk::DescriptorImageInfo imageInfo = {
                        nullptr, texture.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal};

                    vk::WriteDescriptorSet write {};
                    write.dstBinding = cmd.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = vk::DescriptorType::eSampledImage;
                    write.descriptorCount = 1;
                    write.pImageInfo = &imageInfo;

                    _commandBuffer.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics,
                                                        _currentPipeline->getPipelineLayout(),
                                                        0,
                                                        1,
                                                        &write);
                },
                [this](const BindStorageTextureCommand& cmd)
                {
                    auto& texture = *cmd.texture->as<VulkanTexture>();
                    vk::DescriptorImageInfo imageInfo = {
                        nullptr, texture.getImageView(), vk::ImageLayout::eGeneral};

                    vk::WriteDescriptorSet write {};
                    write.dstBinding = cmd.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = vk::DescriptorType::eStorageImage;
                    write.descriptorCount = 1;
                    write.pImageInfo = &imageInfo;

                    _commandBuffer.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics,
                                                        _currentPipeline->getPipelineLayout(),
                                                        0,
                                                        1,
                                                        &write);
                },
                [this](const BindStorageBufferCommand& cmd)
                {
                    auto& buffer = *cmd.buffer->as<VulkanBuffer>();
                    vk::DescriptorBufferInfo bufferInfo {};
                    bufferInfo.buffer = buffer.getBuffer();
                    bufferInfo.offset = 0;
                    bufferInfo.range = buffer.getSize();

                    vk::WriteDescriptorSet write {};
                    write.dstBinding = cmd.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = vk::DescriptorType::eStorageBuffer;
                    write.descriptorCount = 1;
                    write.pBufferInfo = &bufferInfo;

                    _commandBuffer.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics,
                                                        _currentPipeline->getPipelineLayout(),
                                                        0,
                                                        1,
                                                        &write);
                },
                [this](const BindSamplerCommand& cmd)
                {
                    auto& sampler = *cmd.sampler->as<VulkanSampler>();
                    vk::DescriptorImageInfo imageInfo = {
                        sampler.getSampler(), nullptr, vk::ImageLayout::eUndefined};

                    vk::WriteDescriptorSet write {};
                    write.dstBinding = cmd.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = vk::DescriptorType::eSampler;
                    write.descriptorCount = 1;
                    write.pImageInfo = &imageInfo;

                    _commandBuffer.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics,
                                                        _currentPipeline->getPipelineLayout(),
                                                        0,
                                                        1,
                                                        &write);
                }

            },
            command);
    }
}  // namespace exage::Graphics
