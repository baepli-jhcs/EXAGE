#pragma once
#include <vulkan/vulkan.hpp>
#include "fmt/format.h"

#include "Graphics/Commands.h"
#include "Graphics/Texture.h"

namespace exage::Graphics
{
    inline void checkVulkan(vk::Result result)
    {
        debugAssume(result == vk::Result::eSuccess, fmt::format("Vulkan Error: {}", vk::to_string(result)));
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
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;

        if (usage.any(Texture::UsageFlags::eTransferSource))
        {
            flags |= vk::ImageUsageFlagBits::eTransferSrc;
        }

        if (usage.any(Texture::UsageFlags::eTransferDestination))
        {
            flags |= vk::ImageUsageFlagBits::eTransferDst;
        }

        if (usage.any(Texture::UsageFlags::eColorAttachment))
        {
            flags |= vk::ImageUsageFlagBits::eColorAttachment;
        }

        if (usage.any(Texture::UsageFlags::eDepthStencilAttachment))
        {
            flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        }

        return flags;
    }

    [[nodiscard]] constexpr auto toVulkanImageAspectFlags(Texture::Usage usage)
        -> vk::ImageAspectFlags
    {
        vk::ImageAspectFlags flags = vk::ImageAspectFlagBits::eColor;

        if (usage.any(Texture::UsageFlags::eDepthStencilAttachment))
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
            case Texture::Layout::eTransferSrc:
                return vk::ImageLayout::eTransferSrcOptimal;
            case Texture::Layout::eTransferDst:
                return vk::ImageLayout::eTransferDstOptimal;
            case Texture::Layout::ePresent:
                return vk::ImageLayout::ePresentSrcKHR;
            default:
                return vk::ImageLayout::eUndefined;
        }
    }

    [[nodiscard]] constexpr auto toVulkanAccessFlags(Access access) -> vk::AccessFlags
    {
        vk::AccessFlags flags {};

        if (access.any(AccessFlags::eIndirectCommandRead))
        {
            flags |= vk::AccessFlagBits::eIndirectCommandRead;
        }

        if (access.any(AccessFlags::eIndexRead))
        {
            flags |= vk::AccessFlagBits::eIndexRead;
        }

        if (access.any(AccessFlags::eVertexAttributeRead))
        {
            flags |= vk::AccessFlagBits::eVertexAttributeRead;
        }

        if (access.any(AccessFlags::eUniformRead))
        {
            flags |= vk::AccessFlagBits::eUniformRead;
        }

        if (access.any(AccessFlags::eInputAttachmentRead))
        {
            flags |= vk::AccessFlagBits::eInputAttachmentRead;
        }

        if (access.any(AccessFlags::eShaderRead))
        {
            flags |= vk::AccessFlagBits::eShaderRead;
        }

        if (access.any(AccessFlags::eShaderWrite))
        {
            flags |= vk::AccessFlagBits::eShaderWrite;
        }

        if (access.any(AccessFlags::eColorAttachmentRead))
        {
            flags |= vk::AccessFlagBits::eColorAttachmentRead;
        }

        if (access.any(AccessFlags::eColorAttachmentWrite))
        {
            flags |= vk::AccessFlagBits::eColorAttachmentWrite;
        }

        if (access.any(AccessFlags::eDepthStencilAttachmentRead))
        {
            flags |= vk::AccessFlagBits::eDepthStencilAttachmentRead;
        }

        if (access.any(AccessFlags::eDepthStencilAttachmentWrite))
        {
            flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }

        if (access.any(AccessFlags::eTransferRead))
        {
            flags |= vk::AccessFlagBits::eTransferRead;
        }

        if (access.any(AccessFlags::eTransferWrite))
        {
            flags |= vk::AccessFlagBits::eTransferWrite;
        }

        if (access.any(AccessFlags::eHostRead))
        {
            flags |= vk::AccessFlagBits::eHostRead;
        }

        if (access.any(AccessFlags::eHostWrite))
        {
            flags |= vk::AccessFlagBits::eHostWrite;
        }

        if (access.any(AccessFlags::eMemoryRead))
        {
            flags |= vk::AccessFlagBits::eMemoryRead;
        }

        if (access.any(AccessFlags::eMemoryWrite))
        {
            flags |= vk::AccessFlagBits::eMemoryWrite;
        }

        return flags;
    }

    [[nodiscard]] constexpr auto toVulkanPipelineStageFlags(PipelineStage pipelineStages)
        -> vk::PipelineStageFlags
    {
        vk::PipelineStageFlags flags {};

        if (pipelineStages.any(PipelineStageFlags::eTopOfPipe))
        {
            flags |= vk::PipelineStageFlagBits::eTopOfPipe;
        }

        if (pipelineStages.any(PipelineStageFlags::eDrawIndirect))
        {
            flags |= vk::PipelineStageFlagBits::eDrawIndirect;
        }

        if (pipelineStages.any(PipelineStageFlags::eVertexInput))
        {
            flags |= vk::PipelineStageFlagBits::eVertexInput;
        }

        if (pipelineStages.any(PipelineStageFlags::eVertexShader))
        {
            flags |= vk::PipelineStageFlagBits::eVertexShader;
        }

        if (pipelineStages.any(PipelineStageFlags::eTessellationControlShader))
        {
            flags |= vk::PipelineStageFlagBits::eTessellationControlShader;
        }

        if (pipelineStages.any(PipelineStageFlags::eTessellationEvaluationShader))
        {
            flags |= vk::PipelineStageFlagBits::eTessellationEvaluationShader;
        }

        if (pipelineStages.any(PipelineStageFlags::eFragmentShader))
        {
            flags |= vk::PipelineStageFlagBits::eFragmentShader;
        }

        if (pipelineStages.any(PipelineStageFlags::eEarlyFragmentTests))
        {
            flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        }

        if (pipelineStages.any(PipelineStageFlags::eLateFragmentTests))
        {
            flags |= vk::PipelineStageFlagBits::eLateFragmentTests;
        }

        if (pipelineStages.any(PipelineStageFlags::eColorAttachmentOutput))
        {
            flags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
        }

        if (pipelineStages.any(PipelineStageFlags::eComputeShader))
        {
            flags |= vk::PipelineStageFlagBits::eComputeShader;
        }

        if (pipelineStages.any(PipelineStageFlags::eTransfer))
        {
            flags |= vk::PipelineStageFlagBits::eTransfer;
        }

        if (pipelineStages.any(PipelineStageFlags::eBottomOfPipe))
        {
            flags |= vk::PipelineStageFlagBits::eBottomOfPipe;
        }

        if (pipelineStages.any(PipelineStageFlags::eHost))
        {
            flags |= vk::PipelineStageFlagBits::eHost;
        }

        if (pipelineStages.any(PipelineStageFlags::eAllGraphics))
        {
            flags |= vk::PipelineStageFlagBits::eAllGraphics;
        }

        if (pipelineStages.any(PipelineStageFlags::eAllCommands))
        {
            flags |= vk::PipelineStageFlagBits::eAllCommands;
        }

        return flags;
    }
}  // namespace exage::Graphics
