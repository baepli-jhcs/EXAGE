#pragma once
#include "Vulkan/VulkanContext.h"
#include "Graphics/CommandBuffer.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanPrimaryCommandBuffer final : public PrimaryCommandBuffer
    {
    public:
        explicit VulkanPrimaryCommandBuffer(VulkanContext& context) noexcept;
        ~VulkanPrimaryCommandBuffer() override;
        EXAGE_DELETE_COPY(VulkanPrimaryCommandBuffer);
        EXAGE_DEFAULT_MOVE(VulkanPrimaryCommandBuffer);

        [[nodiscard]] auto getCurrentCommandBuffer() const noexcept -> vk::CommandBuffer;


        EXAGE_VULKAN_DERIVED;

    private:
        std::reference_wrapper<VulkanContext> _context;
        std::vector<vk::CommandPool> _commandPools;
        std::vector<vk::CommandBuffer> _commandBuffers;
    };
} // namespace exage::Graphics
