#pragma once

#include "exage/Graphics/Shader.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanShader final : public Shader
    {
      public:
        VulkanShader(VulkanContext& context, const ShaderCreateInfo& createInfo) noexcept;
        ~VulkanShader() override;

        EXAGE_DELETE_COPY(VulkanShader);
        VulkanShader(VulkanShader&& old) noexcept;
        auto operator=(VulkanShader&& old) noexcept -> VulkanShader&;

        [[nodiscard]] auto getShaderModule() const noexcept -> vk::ShaderModule
        {
            return _shaderModule;
        }

        EXAGE_VULKAN_DERIVED;

      private:
        std::reference_wrapper<VulkanContext> _context;
        vk::ShaderModule _shaderModule {};
    };
}  // namespace exage::Graphics
