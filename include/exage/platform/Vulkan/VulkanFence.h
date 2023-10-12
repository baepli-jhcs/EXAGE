#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Fence.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class VulkanFence : public Fence
    {
      public:
        explicit VulkanFence(VulkanContext& context) noexcept;
        ~VulkanFence() override;

        EXAGE_DELETE_COPY(VulkanFence);
        VulkanFence(VulkanFence&& old) noexcept;
        auto operator=(VulkanFence&& old) noexcept -> VulkanFence&;

        void wait() noexcept override;
        void reset() noexcept override;

        [[nodiscard]] auto getState() const noexcept -> State override;

        [[nodiscard]] auto getVulkanFence() const noexcept -> vk::Fence { return _fence; }

        EXAGE_VULKAN_DERIVED;

      private:
        std::reference_wrapper<VulkanContext> _context;
        vk::Fence _fence;
    };
}  // namespace exage::Graphics