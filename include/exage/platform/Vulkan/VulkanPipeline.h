#pragma once

#include "exage/Graphics/Pipeline.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanPipeline final : public Pipeline
    {
      public:
        VulkanPipeline(VulkanContext& context, const PipelineCreateInfo& createInfo) noexcept;
        virtual ~VulkanPipeline();

        EXAGE_DELETE_COPY(VulkanPipeline);

        VulkanPipeline(VulkanPipeline&& old) noexcept;
        auto operator=(VulkanPipeline&& old) noexcept -> VulkanPipeline&;

        [[nodiscard]] auto getPipeline() const noexcept -> vk::Pipeline { return _pipeline; }

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;

        vk::PipelineLayout _pipelineLayout;
        vk::Pipeline _pipeline;
    };
}  // namespace exage::Graphics
