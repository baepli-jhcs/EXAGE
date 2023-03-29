#pragma once

#include "exage/Graphics/Shader.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanShader final : public Shader
    {
      public:
        [[nodiscard]] static auto create(VulkanContext& context,
                                         const ShaderCreateInfo& createInfo) noexcept
            -> tl::expected<VulkanShader, Error>;
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
        VulkanShader(VulkanContext& context, Shader::Stage stage) noexcept
            : _context(context)
            , Shader(stage)
        {
        }
        [[nodiscard]] auto init(const ShaderCreateInfo& createInfo) noexcept
            -> std::optional<Error>;

        [[nodiscard]] auto cacheCompile(const ShaderCreateInfo& createInfo) noexcept
            -> tl::expected<std::vector<uint8_t>, Error>;

        std::reference_wrapper<VulkanContext> _context;
        vk::ShaderModule _shaderModule {};
    };
}  // namespace exage::Graphics
