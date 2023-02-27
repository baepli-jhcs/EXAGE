#pragma once
#include <memory_resource>

#include "Vulkan/VulkanContext.h"
#include "Graphics/CommandBuffer.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanCommandBuffer final : public CommandBuffer
    {
    public:
        [[nodiscard]] static auto create(VulkanContext& context) noexcept
        -> tl::expected<VulkanCommandBuffer, Error>;
        ~VulkanCommandBuffer() override;

        EXAGE_DELETE_COPY(VulkanCommandBuffer);

        VulkanCommandBuffer(VulkanCommandBuffer&& old) noexcept;
        auto operator=(VulkanCommandBuffer&& old) noexcept -> VulkanCommandBuffer&;

        [[nodiscard]] auto begin() noexcept -> std::optional<Error> override;
        [[nodiscard]] auto end() noexcept -> std::optional<Error> override;

        [[nodiscard]] auto getCommandBuffer() const noexcept -> vk::CommandBuffer
        {
            return _commandBuffer;
        }

        EXAGE_VULKAN_DERIVED

    private:
        explicit VulkanCommandBuffer(VulkanContext& context) noexcept;
        [[nodiscard]] auto init() noexcept -> std::optional<Error>;

        void processCommand(const detail::Command& command) noexcept;

        std::reference_wrapper<VulkanContext> _context;
        vk::CommandPool _commandPool;
        vk::CommandBuffer _commandBuffer;

        std::vector<detail::Command> _commands{};
        std::unique_ptr<std::mutex> _commandsMutex = std::make_unique<std::mutex>();
    };
} // namespace exage::Graphics
