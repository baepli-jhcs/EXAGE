#pragma once
#include <vulkan/vulkan.hpp>
#include "Graphics/Texture.h"

namespace exage::Graphics
{
    inline void vulkanAssertMemoryOut(vk::Result result)
    {
        assert(result != vk::Result::eErrorOutOfDeviceMemory && "Out of device memory");
        assert(result != vk::Result::eErrorOutOfHostMemory && "Out of host memory");

        if (result == vk::Result::eErrorOutOfDeviceMemory
            || result == vk::Result::eErrorOutOfHostMemory)
        {
            std::exit(1);
        }
    }

    [[nodiscard]] constexpr auto toVulkanPresentMode(PresentMode presentMode) noexcept
        -> vk::PresentModeKHR
    {
        switch (presentMode)
        {
            case PresentMode::eImmediate:
                return vk::PresentModeKHR::eImmediate;

            case PresentMode::eDoubleBufferVSync:
                return vk::PresentModeKHR::eFifo;

            case PresentMode::eTripleBufferVSync:
                return vk::PresentModeKHR::eMailbox;

            default:
                return vk::PresentModeKHR::eFifo;
        }
    }

    [[nodiscard]] constexpr auto toVulkanTextureFormat(Texture::Format format) noexcept
        -> vk::Format
    {
        switch (format)
        {
            case Texture::Format::eR8:
                return vk::Format::eR8Unorm;
            case Texture::Format::eR16:
                return vk::Format::eR16Unorm;
            case Texture::Format::eRG8:
                return vk::Format::eR8G8Unorm;
            case Texture::Format::eRG16:
                return vk::Format::eR16G16Unorm;
            case Texture::Format::eRGB8:
                return vk::Format::eR8G8B8Unorm;
            case Texture::Format::eRGB16:
                return vk::Format::eR16G16B16Unorm;
            case Texture::Format::eRGBA8:
                return vk::Format::eR8G8B8A8Unorm;
            case Texture::Format::eRGBA16:
                return vk::Format::eR16G16B16A16Unorm;

            case Texture::Format::eR16f:
                return vk::Format::eR16Sfloat;
            case Texture::Format::eRG16f:
                return vk::Format::eR16G16Sfloat;
            case Texture::Format::eRGB16f:
                return vk::Format::eR16G16B16Sfloat;
            case Texture::Format::eRGBA16f:
                return vk::Format::eR16G16B16A16Sfloat;

            case Texture::Format::eR32f:
                return vk::Format::eR32Sfloat;
            case Texture::Format::eRG32f:
                return vk::Format::eR32G32Sfloat;
            case Texture::Format::eRGB32f:
                return vk::Format::eR32G32B32Sfloat;
            case Texture::Format::eRGBA32f:
                return vk::Format::eR32G32B32A32Sfloat;

            case Texture::Format::eDepth24Stencil8:
                return vk::Format::eD24UnormS8Uint;
            case Texture::Format::eDepth32Stencil8:
                return vk::Format::eD32SfloatS8Uint;

            case Texture::Format::eBGRA8:
                return vk::Format::eB8G8R8A8Unorm;

            default:
                return vk::Format::eUndefined;
        }
    }
}
