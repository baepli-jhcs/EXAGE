#pragma once

#include "Core/Core.h"
#include "Graphics/Buffer.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class VulkanBuffer final : public Buffer
    {
      public:
        VulkanBuffer(VulkanContext& context, const BufferCreateInfo& createInfo) noexcept;
        ~VulkanBuffer() override;

        EXAGE_DELETE_COPY(VulkanBuffer);
        VulkanBuffer(VulkanBuffer&& old) noexcept;
        auto operator=(VulkanBuffer&& old) noexcept -> VulkanBuffer&;

        void write(std::span<const std::byte> data, uint64_t offset) noexcept override;
        void read(std::span<std::byte> data, uint64_t offset) const noexcept override;

        [[nodiscard]] auto getBuffer() const noexcept -> vk::Buffer { return _buffer; }
        [[nodiscard]] auto getAllocation() const noexcept -> vma::Allocation { return _allocation; }

        EXAGE_VULKAN_DERIVED;

      private:
        void cleanup() noexcept;

        std::reference_wrapper<VulkanContext> _context;

        vk::Buffer _buffer;
        vma::Allocation _allocation;

        void* _mappedData = nullptr;
    };

}  // namespace exage::Graphics
