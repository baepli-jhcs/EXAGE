#include "Vulkan/VulkanCommandBuffer.h"

#include "Vulkan/VulkanFrameBuffer.h"
#include "Vulkan/VulkanQueue.h"
#include "VulkanTexture.h"

namespace exage::Graphics
{
    auto VulkanCommandBuffer::create(VulkanContext& context) noexcept
        -> tl::expected<VulkanCommandBuffer, Error>
    {
        VulkanCommandBuffer commandBuffer(context);
        std::optional<Error> result = commandBuffer.init();
        if (result.has_value())
        {
            return tl::make_unexpected(result.value());
        }

        return commandBuffer;
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        if (_commandBuffer)
        {
            _context.get().getDevice().freeCommandBuffers(_commandPool, _commandBuffer);
        }

        if (_commandPool)
        {
            _context.get().getDevice().destroyCommandPool(_commandPool);
        }
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& old) noexcept
        : _context(old._context)
    {
        _commandBuffer = old._commandBuffer;
        _commandPool = old._commandPool;
        _commands = std::move(old._commands);
        _commandsMutex = std::move(old._commandsMutex);
        _dataDependencies = std::move(old._dataDependencies);
        _dataDependenciesMutex = std::move(old._dataDependenciesMutex);

        old._commandBuffer = nullptr;
        old._commandPool = nullptr;
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
            _context.get().getDevice().freeCommandBuffers(_commandPool, _commandBuffer);
        }

        if (_commandPool)
        {
            _context.get().getDevice().destroyCommandPool(_commandPool);
        }

        _commandBuffer = old._commandBuffer;
        _commandPool = old._commandPool;
        _commands = std::move(old._commands);
        _commandsMutex = std::move(old._commandsMutex);
        _dataDependencies = std::move(old._dataDependencies);
        _dataDependenciesMutex = std::move(old._dataDependenciesMutex);

        old._commandBuffer = nullptr;
        old._commandPool = nullptr;

        return *this;
    }

    auto VulkanCommandBuffer::begin() noexcept -> std::optional<Error>
    {
        _commands.clear();
        _dataDependencies.clear();
        _commandBuffer.reset();

        _context.get().getDevice().resetCommandPool(_commandPool, {});

        return std::nullopt;
    }

    auto VulkanCommandBuffer::end() noexcept -> std::optional<Error>
    {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        vk::Result result = _commandBuffer.begin(&beginInfo);
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eCommandBufferBeginFailed;
        }

        for (GPUCommand& command : _commands)
        {
            processCommand(command);
        }

        _commandBuffer.end();
        return std::nullopt;
    }

    void VulkanCommandBuffer::submitCommand(GPUCommand command) noexcept
    {
        if (std::holds_alternative<TextureBarrier>(command))
        {
            TextureBarrier& barrier = std::get<TextureBarrier>(command);
            barrier._oldLayout = barrier.texture->_layout;
            barrier.texture->_layout = barrier.newLayout;
        }

        std::lock_guard<std::mutex> lock(*_commandsMutex);
        _commands.push_back(command);
    }

    void VulkanCommandBuffer::insertDataDependency(DataDependency dependency) noexcept
    {
        std::lock_guard<std::mutex> lock(*_dataDependenciesMutex);
        _dataDependencies.push_back(dependency);
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanContext& context) noexcept
        : _context(context)
    {
    }

    auto VulkanCommandBuffer::init() noexcept -> std::optional<Error>
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = _context.get().getQueueIndex();
        commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        vk::Result result = _context.get().getDevice().createCommandPool(
            &commandPoolCreateInfo, nullptr, &_commandPool);

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eCommandPoolCreationFailed;
        }

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        commandBufferAllocateInfo.commandPool = _commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;

        result = _context.get().getDevice().allocateCommandBuffers(&commandBufferAllocateInfo,
                                                                   &_commandBuffer);

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eCommandBufferCreationFailed;
        }

        return std::nullopt;
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
                [this](const TextureBarrier& barrier)
                {
                    auto& texture = *barrier.texture->as<VulkanTexture>();

                    vk::ImageMemoryBarrier imageMemoryBarrier;
                    imageMemoryBarrier.oldLayout = toVulkanImageLayout(barrier._oldLayout);
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
                [this](const BlitCommand& copy)
                {
                    auto& srcTexture = *copy.srcTexture->as<VulkanTexture>();
                    auto& dstTexture = *copy.dstTexture->as<VulkanTexture>();

                    assert(srcTexture.getLayout() == Texture::Layout::eTransferSrc);
                    assert(dstTexture.getLayout() == Texture::Layout::eTransferDst);

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
                        vk::Offset3D {static_cast<int32_t>(copy.srcOffset.x + copy.extent.x),
                                      static_cast<int32_t>(copy.srcOffset.y + copy.extent.y),
                                      static_cast<int32_t>(copy.srcOffset.z + copy.extent.z)};
                    imageBlit.dstSubresource.aspectMask =
                        toVulkanImageAspectFlags(dstTexture.getUsage());
                    imageBlit.dstSubresource.mipLevel = copy.dstMipLevel;
                    imageBlit.dstSubresource.baseArrayLayer = copy.dstFirstLayer;
                    imageBlit.dstSubresource.layerCount = copy.layerCount;
                    imageBlit.dstOffsets[0] = vk::Offset3D {static_cast<int32_t>(copy.dstOffset.x),
                                                            static_cast<int32_t>(copy.dstOffset.y),
                                                            static_cast<int32_t>(copy.dstOffset.z)};
                    imageBlit.dstOffsets[1] =
                        vk::Offset3D {static_cast<int32_t>(copy.dstOffset.x + copy.extent.x),
                                      static_cast<int32_t>(copy.dstOffset.y + copy.extent.y),
                                      static_cast<int32_t>(copy.dstOffset.z + copy.extent.z)};

                    _commandBuffer.blitImage(srcTexture.getImage(),
                                             vk::ImageLayout::eTransferSrcOptimal,
                                             dstTexture.getImage(),
                                             vk::ImageLayout::eTransferDstOptimal,
                                             1,
                                             &imageBlit,
                                             vk::Filter::eLinear);
                },
                [this](const UserDefinedCommand& cmd) { cmd.commandFunction(*this); },
                [this](const ViewportCommand& cmd)
                {
                    vk::Viewport viewport {};
                    viewport.x = cmd.offset.x;
                    viewport.y = cmd.offset.y;
                    viewport.width = cmd.extent.x;
                    viewport.height = cmd.extent.y;
                    viewport.maxDepth = 1.0;
                    viewport.minDepth = 0.0;

                    _commandBuffer.setViewport(0, viewport);
                },
                [this](const ScissorCommand& cmd)
                {
                    vk::Rect2D rect {};
                    rect.offset.x = cmd.offset.x;
                    rect.offset.y = cmd.offset.y;
                    rect.extent.width = cmd.extent.x;
                    rect.extent.height = cmd.extent.y;
                    _commandBuffer.setScissor(0, rect);
                },
                [this](const ClearTextureCommand& cmd)
                {
                    auto& texture = *cmd.texture->as<VulkanTexture>();

                    assert(texture.getLayout() == Texture::Layout::eTransferDst);
                    assert(texture.getUsage().none(Texture::UsageFlags::eDepthStencilAttachment));

                    std::array<float, 4> color = {
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
                    assert(textures.size() == cmd.clearColors.size());

                    vk::RenderingInfo renderingInfo {};
                    glm::uvec2 extent = frameBuffer.getExtent();
                    renderingInfo.renderArea =
                        vk::Rect2D {vk::Offset2D {0, 0}, vk::Extent2D {extent.x, extent.y}};

                    std::vector<vk::RenderingAttachmentInfo> info;
                    info.reserve(textures.size());

                    for (size_t i = 0; i < textures.size(); i++)
                    {
                        auto& vulkanTexture = *textures[i]->as<VulkanTexture>();
                        auto& clearColor = cmd.clearColors[i];

                        vk::RenderingAttachmentInfo attachmentInfo {};
                        attachmentInfo.imageView = vulkanTexture.getImageView();
                        attachmentInfo.imageLayout = toVulkanImageLayout(vulkanTexture.getLayout());
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

                    std::shared_ptr<Texture> depthTexture = frameBuffer.getDepthStencilTexture();
                    if (depthTexture)
                    {
                        auto& vulkanTexture = *depthTexture->as<VulkanTexture>();
                        auto& clearDepth = cmd.clearDepth;

                        vk::RenderingAttachmentInfo depthAttachmentInfo {};
                        depthAttachmentInfo.imageView = vulkanTexture.getImageView();
                        depthAttachmentInfo.imageLayout =
                            toVulkanImageLayout(vulkanTexture.getLayout());
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
                [this](const EndRenderingCommand& cmd) { _commandBuffer.endRendering(); }

            },
            command);
    }
}  // namespace exage::Graphics
