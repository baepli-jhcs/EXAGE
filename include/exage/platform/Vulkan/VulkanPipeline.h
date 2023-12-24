#pragma once

#include "exage/Graphics/Pipeline.h"
#include "exage/platform/Vulkan/VulkanContext.h"
#include "exage/platform/Vulkan/VulkanResourceManager.h"

namespace exage::Graphics
{
    class VulkanGraphicsPipeline final : public GraphicsPipeline
    {
      public:
        VulkanGraphicsPipeline(VulkanContext& context,
                               const GraphicsPipelineCreateInfo& createInfo) noexcept;
        virtual ~VulkanGraphicsPipeline();

        EXAGE_DELETE_COPY(VulkanGraphicsPipeline);

        VulkanGraphicsPipeline(VulkanGraphicsPipeline&& old) noexcept;
        auto operator=(VulkanGraphicsPipeline&& old) noexcept -> VulkanGraphicsPipeline&;

        [[nodiscard]] auto getPipeline() const noexcept -> vk::Pipeline { return _pipeline; }
        [[nodiscard]] auto getPipelineLayout() const noexcept -> vk::PipelineLayout
        {
            return _pipelineLayout;
        }

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;

        vk::PipelineLayout _pipelineLayout;
        vk::Pipeline _pipeline;
    };

    class VulkanComputePipeline final : public ComputePipeline
    {
      public:
        VulkanComputePipeline(VulkanContext& context,
                              const ComputePipelineCreateInfo& createInfo) noexcept;
        virtual ~VulkanComputePipeline();

        EXAGE_DELETE_COPY(VulkanComputePipeline);

        VulkanComputePipeline(VulkanComputePipeline&& old) noexcept;
        auto operator=(VulkanComputePipeline&& old) noexcept -> VulkanComputePipeline&;

        [[nodiscard]] auto getPipeline() const noexcept -> vk::Pipeline { return _pipeline; }
        [[nodiscard]] auto getPipelineLayout() const noexcept -> vk::PipelineLayout
        {
            return _pipelineLayout;
        }

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;

        vk::PipelineLayout _pipelineLayout;
        vk::Pipeline _pipeline;
    };
}  // namespace exage::Graphics
