#pragma once
#include <vulkan/vulkan.hpp>

#include "Graphics/Commands.h"
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

    [[nodiscard]] constexpr auto toVulkanFormat(Texture::Format format) noexcept -> vk::Format
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

    [[nodiscard]] constexpr auto toVulkanImageType(Texture::Type type) noexcept -> vk::ImageType
    {
        switch (type)
        {
            case Texture::Type::e1D:
                return vk::ImageType::e1D;
            case Texture::Type::e2D:
                return vk::ImageType::e2D;
            case Texture::Type::e3D:
                return vk::ImageType::e3D;
            case Texture::Type::eCube:
                return vk::ImageType::e2D;
            default:
                return vk::ImageType::e1D;
        }
    }

    [[nodiscard]] constexpr auto toVulkanImageViewType(Texture::Type type) noexcept
    -> vk::ImageViewType
    {
        switch (type)
        {
            case Texture::Type::e1D:
                return vk::ImageViewType::e1D;
            case Texture::Type::e2D:
                return vk::ImageViewType::e2D;
            case Texture::Type::e3D:
                return vk::ImageViewType::e3D;
            case Texture::Type::eCube:
                return vk::ImageViewType::eCube;
            default:
                return vk::ImageViewType::e1D;
        }
    }

    [[nodiscard]] constexpr auto toVulkanImageUsageFlags(Texture::Usage usage) noexcept
    -> vk::ImageUsageFlags
    {
        vk::ImageUsageFlags flags =
            vk::ImageUsageFlagBits::eSampled & vk::ImageUsageFlagBits::eStorage;

        if (usage & Texture::Usage::eTransferSource)
        {
            flags |= vk::ImageUsageFlagBits::eTransferSrc;
        }

        if (usage & Texture::Usage::eTransferDestination)
        {
            flags |= vk::ImageUsageFlagBits::eTransferDst;
        }

        if (usage & Texture::Usage::eColorAttachment)
        {
            flags |= vk::ImageUsageFlagBits::eColorAttachment;
        }

        if (usage & Texture::Usage::eDepthStencilAttachment)
        {
            flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        }

        return flags;
    }

    [[nodiscard]] constexpr auto toVulkanImageAspectFlags(Texture::Usage usage)
    -> vk::ImageAspectFlags
    {
        vk::ImageAspectFlags flags = vk::ImageAspectFlagBits::eColor;

        if (usage & Texture::Usage::eDepthStencilAttachment)
        {
            flags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        }

        return flags;
    }

    [[nodiscard]] constexpr auto toVulkanFilter(Sampler::Filter filter) -> vk::Filter
    {
        switch (filter)
        {
            case Sampler::Filter::eNearest:
                return vk::Filter::eNearest;
            case Sampler::Filter::eLinear:
                return vk::Filter::eLinear;
            default:
                return vk::Filter::eNearest;
        }
    }

    [[nodiscard]] constexpr auto toVulkanSamplerMipmapMode(Sampler::MipmapMode mipmapMode) noexcept
    -> vk::SamplerMipmapMode
    {
        switch (mipmapMode)
        {
            case Sampler::MipmapMode::eNearest:
                return vk::SamplerMipmapMode::eNearest;
            case Sampler::MipmapMode::eLinear:
                return vk::SamplerMipmapMode::eLinear;
            default:
                return vk::SamplerMipmapMode::eNearest;
        }
    }

    [[nodiscard]] constexpr auto toVulkanImageLayout(Texture::Layout layout) noexcept
    -> vk::ImageLayout
    {
        switch (layout)
        {
            case Texture::Layout::eUndefined:
                return vk::ImageLayout::eUndefined;
            case Texture::Layout::eColorAttachment:
                return vk::ImageLayout::eColorAttachmentOptimal;
            case Texture::Layout::eDepthStencilAttachment:
                return vk::ImageLayout::eDepthStencilAttachmentOptimal;
            case Texture::Layout::eShaderReadOnly:
                return vk::ImageLayout::eShaderReadOnlyOptimal;
            case Texture::Layout::eTransferSource:
                return vk::ImageLayout::eTransferSrcOptimal;
            case Texture::Layout::eTransferDestination:
                return vk::ImageLayout::eTransferDstOptimal;
            case Texture::Layout::ePresent:
                return vk::ImageLayout::ePresentSrcKHR;
            default:
                return vk::ImageLayout::eUndefined;
        }
    }

    [[nodiscard]] constexpr auto toVulkanAccessFlags(Access access) -> vk::AccessFlags
    {
        vk::AccessFlags flags{};

        if (access & Access::eIndirectCommandRead)
        {
            flags |= vk::AccessFlagBits::eIndirectCommandRead;
        }

        if (access & Access::eIndexRead)
        {
            flags |= vk::AccessFlagBits::eIndexRead;
        }

        if (access & Access::eVertexAttributeRead)
        {
            flags |= vk::AccessFlagBits::eVertexAttributeRead;
        }

        if (access & Access::eUniformRead)
        {
            flags |= vk::AccessFlagBits::eUniformRead;
        }

        if (access & Access::eInputAttachmentRead)
        {
            flags |= vk::AccessFlagBits::eInputAttachmentRead;
        }

        if (access & Access::eShaderRead)
        {
            flags |= vk::AccessFlagBits::eShaderRead;
        }

        if (access & Access::eShaderWrite)
        {
            flags |= vk::AccessFlagBits::eShaderWrite;
        }

        if (access & Access::eColorAttachmentRead)
        {
            flags |= vk::AccessFlagBits::eColorAttachmentRead;
        }

        if (access & Access::eColorAttachmentWrite)
        {
            flags |= vk::AccessFlagBits::eColorAttachmentWrite;
        }

        if (access & Access::eDepthStencilAttachmentRead)
        {
            flags |= vk::AccessFlagBits::eDepthStencilAttachmentRead;
        }

        if (access & Access::eDepthStencilAttachmentWrite)
        {
            flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }

        if (access & Access::eTransferRead)
        {
            flags |= vk::AccessFlagBits::eTransferRead;
        }

        if (access & Access::eTransferWrite)
        {
            flags |= vk::AccessFlagBits::eTransferWrite;
        }

        if (access & Access::eHostRead)
        {
            flags |= vk::AccessFlagBits::eHostRead;
        }

        if (access & Access::eHostWrite)
        {
            flags |= vk::AccessFlagBits::eHostWrite;
        }

        if (access & Access::eMemoryRead)
        {
            flags |= vk::AccessFlagBits::eMemoryRead;
        }

        if (access & Access::eMemoryWrite)
        {
            flags |= vk::AccessFlagBits::eMemoryWrite;
        }

        return flags;
    }

    constexpr auto toVulkanPipelineStageFlags(PipelineStage pipelineStage) -> vk::PipelineStageFlags
    {
        vk::PipelineStageFlags flags{};

        if (pipelineStage & PipelineStage::eTopOfPipe)
        {
            flags |= vk::PipelineStageFlagBits::eTopOfPipe;
        }

        if (pipelineStage & PipelineStage::eDrawIndirect)
        {
            flags |= vk::PipelineStageFlagBits::eDrawIndirect;
        }

        if (pipelineStage & PipelineStage::eVertexInput)
        {
            flags |= vk::PipelineStageFlagBits::eVertexInput;
        }

        if (pipelineStage & PipelineStage::eVertexShader)
        {
            flags |= vk::PipelineStageFlagBits::eVertexShader;
        }

        if (pipelineStage & PipelineStage::eTessellationControlShader)
        {
            flags |= vk::PipelineStageFlagBits::eTessellationControlShader;
        }

        if (pipelineStage & PipelineStage::eTessellationEvaluationShader)
        {
            flags |= vk::PipelineStageFlagBits::eTessellationEvaluationShader;
        }

        if (pipelineStage & PipelineStage::eFragmentShader)
        {
            flags |= vk::PipelineStageFlagBits::eFragmentShader;
        }

        if (pipelineStage & PipelineStage::eEarlyFragmentTests)
        {
            flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        }

        if (pipelineStage & PipelineStage::eLateFragmentTests)
        {
            flags |= vk::PipelineStageFlagBits::eLateFragmentTests;
        }

        if (pipelineStage & PipelineStage::eColorAttachmentOutput)
        {
            flags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
        }

        if (pipelineStage & PipelineStage::eComputeShader)
        {
            flags |= vk::PipelineStageFlagBits::eComputeShader;
        }

        if (pipelineStage & PipelineStage::eTransfer)
        {
            flags |= vk::PipelineStageFlagBits::eTransfer;
        }

        if (pipelineStage & PipelineStage::eBottomOfPipe)
        {
            flags |= vk::PipelineStageFlagBits::eBottomOfPipe;
        }

        if (pipelineStage & PipelineStage::eHost)
        {
            flags |= vk::PipelineStageFlagBits::eHost;
        }

        if (pipelineStage & PipelineStage::eAllGraphics)
        {
            flags |= vk::PipelineStageFlagBits::eAllGraphics;
        }

        if (pipelineStage & PipelineStage::eAllCommands)
        {
            flags |= vk::PipelineStageFlagBits::eAllCommands;
        }

        return flags;
    }

} // namespace exage::Graphics
