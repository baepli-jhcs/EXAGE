#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/BindlessResources.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Sampler.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/utils/classes.h"

namespace exage::Graphics
{
    class VulkanSampler final : public Sampler
    {
      public:
        VulkanSampler(VulkanContext& context, const SamplerCreateInfo& createInfo) noexcept;
        ~VulkanSampler() override;

        VulkanSampler(VulkanSampler&&) noexcept;
        auto operator=(VulkanSampler&&) noexcept -> VulkanSampler&;

        [[nodiscard]] auto getSampler() const noexcept -> vk::Sampler { return _sampler; }

        EXAGE_DELETE_COPY(VulkanSampler);

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;
        vk::Sampler _sampler;
    };
}  // namespace exage::Graphics