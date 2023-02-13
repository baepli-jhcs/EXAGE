#pragma once
#include "Vulkan/VulkanContext.h"
#include "Graphics/CommandBuffer.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanPrimaryCommandBuffer final : public QueueCommandBuffer
    {
    public:
        explicit VulkanPrimaryCommandBuffer(VulkanContext& context) noexcept;
        ~VulkanPrimaryCommandBuffer() override;
        EXAGE_DELETE_COPY(VulkanPrimaryCommandBuffer);
        EXAGE_DEFAULT_MOVE(VulkanPrimaryCommandBuffer);

        std::optional<Error> beginFrame() noexcept override;
        std::optional<Error> endFrame() noexcept override;

        [[nodiscard]] auto getCurrentCommandBuffer() const noexcept -> vk::CommandBuffer;

        EXAGE_VULKAN_DERIVED;

    private:
        std::reference_wrapper<VulkanContext> _context;
        std::vector<vk::CommandPool> _commandPools;
        std::vector<vk::CommandBuffer> _commandBuffers;
    };

    class EXAGE_EXPORT VulkanTemporaryCommandBuffer final : public TemporaryCommandBuffer
    {
    public:
        explicit VulkanTemporaryCommandBuffer(VulkanContext& context) noexcept;
        ~VulkanTemporaryCommandBuffer() override;
        EXAGE_DELETE_COPY(VulkanTemporaryCommandBuffer);
        EXAGE_DEFAULT_MOVE(VulkanTemporaryCommandBuffer);

        EXAGE_VULKAN_DERIVED;

    private:
        std::reference_wrapper<VulkanContext> _context;
        vk::CommandPool _commandPool;
        vk::CommandBuffer _commandBuffer;
    };
} // namespace exage::Graphics