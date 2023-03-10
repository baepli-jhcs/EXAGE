#include "Vulkan/VulkanCommandBuffer.h"

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
        *this = std::move(old);
    }

    auto VulkanCommandBuffer::operator=(VulkanCommandBuffer&& old) noexcept -> VulkanCommandBuffer&
    {
        _commandBuffer = old._commandBuffer;
        _commandPool = old._commandPool;
        _commands = std::move(old._commands);
        _commandsMutex = std::move(old._commandsMutex);

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
            barrier._oldLayout = barrier.texture -> _layout;
            barrier.texture -> _layout = barrier.newLayout;
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
                        vk::ImageAspectFlagBits::eColor;
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
                    imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    imageBlit.srcSubresource.mipLevel = copy.srcMipLevel;
                    imageBlit.srcSubresource.baseArrayLayer = copy.srcLayer;
                    imageBlit.srcSubresource.layerCount = 1;
                    imageBlit.srcOffsets[0] = vk::Offset3D {static_cast<int32_t>(copy.srcOffset.x),
                                                            static_cast<int32_t>(copy.srcOffset.y),
                                                            static_cast<int32_t>(copy.srcOffset.z)};
                    imageBlit.srcOffsets[1] =
                        vk::Offset3D {static_cast<int32_t>(copy.srcOffset.x + copy.extent.x),
                                      static_cast<int32_t>(copy.srcOffset.y + copy.extent.y),
                                      static_cast<int32_t>(copy.srcOffset.z + copy.extent.z)};
                    imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    imageBlit.dstSubresource.mipLevel = copy.dstMipLevel;
                    imageBlit.dstSubresource.baseArrayLayer = copy.dstLayer;
                    imageBlit.dstSubresource.layerCount = 1;
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
                [this](const UserDefinedCommand& cmd) { cmd.commandFunction(*this); }},
            command);
    }
}  // namespace exage::Graphics
