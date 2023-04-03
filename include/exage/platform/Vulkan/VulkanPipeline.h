#pragma once

#include "exage/Graphics/Pipeline.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
	class EXAGE_EXPORT VulkanPipeline final : public Pipeline
	{
	  public:
		VulkanPipeline(VulkanContext& context, const PipelineCreateInfo& createInfo) noexcept;
		virtual ~VulkanPipeline() = default;

		EXAGE_DELETE_COPY(VulkanPipeline);
		EXAGE_DEFAULT_MOVE(VulkanPipeline);

		[[nodiscard]] auto getPipeline() const noexcept -> vk::Pipeline { return _pipeline; }

		EXAGE_VULKAN_DERIVED

	  private:
		std::reference_wrapper<VulkanContext> _context;

		vk::Pipeline _pipeline;
	};
}  // namespace exage::Graphics
