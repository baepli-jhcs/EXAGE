#include <fstream>

#include "exage/platform/Vulkan/VulkanShader.h"

namespace exage::Graphics
{
    VulkanShader::VulkanShader(VulkanContext& context, const ShaderCreateInfo& createInfo) noexcept
        : Shader(createInfo.stage)
        , _context(context)
    {
        vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
        shaderModuleCreateInfo.codeSize = createInfo.irCode.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode = createInfo.irCode.data();
        checkVulkan(context.getDevice().createShaderModule(
            &shaderModuleCreateInfo, nullptr, &_shaderModule));
    }

    VulkanShader::~VulkanShader()
    {
        if (_shaderModule)
        {
            _context.get().getDevice().destroyShaderModule(_shaderModule);
        }
    }

    VulkanShader::VulkanShader(VulkanShader&& old) noexcept
        : Shader(std::move(old))
        , _context(old._context)
        , _shaderModule(std::exchange(old._shaderModule, nullptr))
    {
    }

    auto VulkanShader::operator=(VulkanShader&& old) noexcept -> VulkanShader&
    {
        if (this != &old)
        {
            Shader::operator=(std::move(old));
            _context = old._context;
            _shaderModule = std::exchange(old._shaderModule, nullptr);
        }
        return *this;
    }
}  // namespace exage::Graphics
