#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanQueue.h"

namespace exage::Graphics
{
    auto VulkanCommandBuffer::create(
        VulkanContext& context) noexcept -> tl::expected<VulkanCommandBuffer, Error>
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
        _context.get().getDevice().freeCommandBuffers(_commandPool, _commandBuffer);
        _context.get().getDevice().destroyCommandPool(_commandPool);
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

        for (detail::Command& command : _commands)
        {
            processCommand(command);
        }

        _commandBuffer.end();
        return std::nullopt;
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanContext& context) noexcept
        : _context(context) { }

    auto VulkanCommandBuffer::init() noexcept -> std::optional<Error>
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = _context.get().getQueueIndex();
        commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        vk::Result result =
            _context.get().getDevice().createCommandPool(&commandPoolCreateInfo,
                                                         nullptr,
                                                         &_commandPool);

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eCommandPoolCreationFailed;
        }

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        commandBufferAllocateInfo.commandPool = _commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;

        result =
            _context.get().getDevice().allocateCommandBuffers(
                &commandBufferAllocateInfo,
                &_commandBuffer);

        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eCommandBufferCreationFailed;
        }

        return std::nullopt;
    }

    void VulkanCommandBuffer::processCommand(const detail::Command& command) noexcept
    {
        std::visit(
            Overload{
                [this](const detail::DrawCommand& draw)
                {
                    _commandBuffer.draw(draw.vertexCount,
                                        draw.instanceCount,
                                        draw.firstVertex,
                                        draw.firstInstance);
                }
            },
            command);
    }
} // namespace exage::Graphics
