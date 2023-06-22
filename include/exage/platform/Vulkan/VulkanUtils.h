#pragma once

#include "VKinclude.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Graphics/Sampler.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/Graphics/Texture.h"
#include "fmt/format.h"

namespace exage::Graphics
{
    inline void checkVulkan(vk::Result result)
    {
        debugAssume(result == vk::Result::eSuccess,
                    fmt::format("Vulkan Error: {}", vk::to_string(result)));
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

    [[nodiscard]] constexpr auto toVulkanFormat(Format format) noexcept -> vk::Format
    {
        switch (format)
        {
            case Format::eR8:
                return vk::Format::eR8Unorm;
            case Format::eR16:
                return vk::Format::eR16Unorm;
            case Format::eRG8:
                return vk::Format::eR8G8Unorm;
            case Format::eRG16:
                return vk::Format::eR16G16Unorm;
            case Format::eRGBA8:
                return vk::Format::eR8G8B8A8Unorm;
            case Format::eRGBA16:
                return vk::Format::eR16G16B16A16Unorm;

            case Format::eR16f:
                return vk::Format::eR16Sfloat;
            case Format::eRG16f:
                return vk::Format::eR16G16Sfloat;
            case Format::eRGBA16f:
                return vk::Format::eR16G16B16A16Sfloat;

            case Format::eR32f:
                return vk::Format::eR32Sfloat;
            case Format::eRG32f:
                return vk::Format::eR32G32Sfloat;
            case Format::eRGBA32f:
                return vk::Format::eR32G32B32A32Sfloat;

            case Format::eDepth24Stencil8:
                return vk::Format::eD24UnormS8Uint;
            case Format::eDepth32Stencil8:
                return vk::Format::eD32SfloatS8Uint;

            case Format::eBGRA8:
                return vk::Format::eB8G8R8A8Unorm;

            // Compressed Formats
            case Format::eBC1RGBA8:
                return vk::Format::eBc1RgbaUnormBlock;
            case Format::eBC3RGBA8:
                return vk::Format::eBc3UnormBlock;
            case Format::eBC4R8:
                return vk::Format::eBc4UnormBlock;
            case Format::eBC5RG8:
                return vk::Format::eBc5UnormBlock;
            case Format::eBC7RGBA8:
                return vk::Format::eBc7UnormBlock;
            case Format::eASTC4x4RGBA8:
                return vk::Format::eAstc4x4UnormBlock;
            case Format::eASTC6x6RGBA8:
                return vk::Format::eAstc6x6UnormBlock;
            case Format::eETC2RGBA8:
                return vk::Format::eEtc2R8G8B8A8UnormBlock;

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

    [[nodiscard]] constexpr auto toVulkanImageViewType(Texture::Type type, uint32_t layers) noexcept
        -> vk::ImageViewType
    {
        if (layers > 1)
        {
            switch (type)
            {
                case Texture::Type::e1D:
                    return vk::ImageViewType::e1DArray;
                case Texture::Type::e2D:
                    return vk::ImageViewType::e2DArray;
                case Texture::Type::e3D:
                    return vk::ImageViewType::e3D;
                case Texture::Type::eCube:
                    if (layers == 6)
                    {
                        return vk::ImageViewType::eCube;
                    }
                    else
                    {
                        return vk::ImageViewType::eCubeArray;
                    }
                default:
                    return vk::ImageViewType::e1DArray;
            }
        }
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
        vk::ImageUsageFlags flags {};

        if (usage.any(Texture::UsageFlags::eSampled))
        {
            flags |= vk::ImageUsageFlagBits::eSampled;
        }

        if (usage.any(Texture::UsageFlags::eStorage))
        {
            flags |= vk::ImageUsageFlagBits::eStorage;
        }

        if (usage.any(Texture::UsageFlags::eTransferSrc))
        {
            flags |= vk::ImageUsageFlagBits::eTransferSrc;
        }

        if (usage.any(Texture::UsageFlags::eTransferDst))
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
            case Texture::Layout::eStorage:
                return vk::ImageLayout::eGeneral;
            case Texture::Layout::eDepthStencilReadOnly:
                return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
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

    [[nodiscard]] constexpr auto toVmaAllocationCreateFlags(Buffer::MapMode mapMode, bool cached)
        -> vma::AllocationCreateFlags
    {
        vma::AllocationCreateFlags flags {};

        bool ifOptimal = mapMode == Buffer::MapMode::eIfOptimal;
        bool mapped = mapMode == Buffer::MapMode::eMapped;

        if (mapped || ifOptimal)
        {
            if (cached)
            {
                flags |= vma::AllocationCreateFlagBits::eHostAccessRandom;
            }
            else
            {
                flags |= vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
            }

            flags |= vma::AllocationCreateFlagBits::eMapped;
        }

        if (ifOptimal)
        {
            flags |= vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead;
        }

        return flags;
    }

    [[nodiscard]] constexpr auto toVulkanVertexInputRate(VertexDescription::InputRate rate)
        -> vk::VertexInputRate
    {
        switch (rate)
        {
            case VertexDescription::InputRate::eVertex:
                return vk::VertexInputRate::eVertex;
            case VertexDescription::InputRate::eInstance:
                return vk::VertexInputRate::eInstance;
        }
        return vk::VertexInputRate::eVertex;
    }

    [[nodiscard]] constexpr auto toVulkanPolygonMode(Pipeline::PolygonMode mode) -> vk::PolygonMode
    {
        switch (mode)
        {
            case Pipeline::PolygonMode::eFill:
                return vk::PolygonMode::eFill;
            case Pipeline::PolygonMode::eLine:
                return vk::PolygonMode::eLine;
            case Pipeline::PolygonMode::ePoint:
                return vk::PolygonMode::ePoint;
        }
        return vk::PolygonMode::eFill;
    }

    [[nodiscard]] constexpr auto toVulkanCullModeFlags(Pipeline::CullMode mode) -> vk::CullModeFlags
    {
        switch (mode)
        {
            case Pipeline::CullMode::eNone:
                return vk::CullModeFlagBits::eNone;
            case Pipeline::CullMode::eFront:
                return vk::CullModeFlagBits::eFront;
            case Pipeline::CullMode::eBack:
                return vk::CullModeFlagBits::eBack;
        }
        return vk::CullModeFlagBits::eNone;
    }

    [[nodiscard]] constexpr auto toVulkanFormat(uint32_t components, VertexAttribute::Type type)
        -> vk::Format
    {
        switch (components)
        {
            case 1:
                switch (type)
                {
                    case VertexAttribute::Type::eFloat:
                        return vk::Format::eR32Sfloat;
                    case VertexAttribute::Type::eU32:
                        return vk::Format::eR32Uint;
                    case VertexAttribute::Type::eI8:
                        return vk::Format::eR8Sint;
                    case VertexAttribute::Type::eI32:
                        return vk::Format::eR32Sint;
                }
                return vk::Format::eR32Uint;
            case 2:
                switch (type)
                {
                    case VertexAttribute::Type::eFloat:
                        return vk::Format::eR32G32Sfloat;
                    case VertexAttribute::Type::eU32:
                        return vk::Format::eR32G32Uint;
                    case VertexAttribute::Type::eI8:
                        return vk::Format::eR8G8Sint;
                    case VertexAttribute::Type::eI32:
                        return vk::Format::eR32G32Sint;
                }
                return vk::Format::eR32G32Uint;
            case 3:
                switch (type)
                {
                    case VertexAttribute::Type::eFloat:
                        return vk::Format::eR32G32B32Sfloat;
                    case VertexAttribute::Type::eU32:
                        return vk::Format::eR32G32B32Uint;
                    case VertexAttribute::Type::eI8:
                        return vk::Format::eR8G8B8Sint;
                    case VertexAttribute::Type::eI32:
                        return vk::Format::eR32G32B32Sint;
                }
                return vk::Format::eR32G32B32Uint;
            case 4:
                switch (type)
                {
                    case VertexAttribute::Type::eFloat:
                        return vk::Format::eR32G32B32A32Sfloat;
                    case VertexAttribute::Type::eU32:
                        return vk::Format::eR32G32B32A32Uint;
                    case VertexAttribute::Type::eI8:
                        return vk::Format::eR8G8B8A8Sint;
                    case VertexAttribute::Type::eI32:
                        return vk::Format::eR32G32B32A32Sint;
                }
                return vk::Format::eR32G32B32A32Uint;
            default:
                break;
        }

        return vk::Format::eUndefined;
    }

    [[nodiscard]] constexpr auto toVulkanFrontFace(Pipeline::FrontFace face) -> vk::FrontFace
    {
        switch (face)
        {
            case Pipeline::FrontFace::eClockwise:
                return vk::FrontFace::eClockwise;
            case Pipeline::FrontFace::eCounterClockwise:
                return vk::FrontFace::eCounterClockwise;
        }
        return vk::FrontFace::eClockwise;
    }

    [[nodiscard]] constexpr auto toVulkanCompareOp(Pipeline::CompareOperation op) -> vk::CompareOp
    {
        switch (op)
        {
            case Pipeline::CompareOperation::eLess:
                return vk::CompareOp::eLess;
            case Pipeline::CompareOperation::eEqual:
                return vk::CompareOp::eEqual;
            case Pipeline::CompareOperation::eLessOrEqual:
                return vk::CompareOp::eLessOrEqual;
            case Pipeline::CompareOperation::eGreater:
                return vk::CompareOp::eGreater;
            case Pipeline::CompareOperation::eNotEqual:
                return vk::CompareOp::eNotEqual;
            case Pipeline::CompareOperation::eGreaterOrEqual:
                return vk::CompareOp::eGreaterOrEqual;
        }
        return vk::CompareOp::eAlways;
    }

    [[nodiscard]] constexpr auto toVulkanStencilOp(Pipeline::DepthStencilState::StencilOperation op)
        -> vk::StencilOp
    {
        switch (op)
        {
            case Pipeline::DepthStencilState::StencilOperation::eKeep:
                return vk::StencilOp::eKeep;
            case Pipeline::DepthStencilState::StencilOperation::eZero:
                return vk::StencilOp::eZero;
            case Pipeline::DepthStencilState::StencilOperation::eReplace:
                return vk::StencilOp::eReplace;
            case Pipeline::DepthStencilState::StencilOperation::eIncrementClamp:
                return vk::StencilOp::eIncrementAndClamp;
            case Pipeline::DepthStencilState::StencilOperation::eDecrementClamp:
                return vk::StencilOp::eDecrementAndClamp;
            case Pipeline::DepthStencilState::StencilOperation::eInvert:
                return vk::StencilOp::eInvert;
            case Pipeline::DepthStencilState::StencilOperation::eIncrementWrap:
                return vk::StencilOp::eIncrementAndWrap;
            case Pipeline::DepthStencilState::StencilOperation::eDecrementWrap:
                return vk::StencilOp::eDecrementAndWrap;
        }
        return vk::StencilOp::eKeep;
    }

    [[nodiscard]] constexpr auto toVulkanStencilOpState(
        Pipeline::DepthStencilState::StencilOperationState opState) -> vk::StencilOpState
    {
        vk::StencilOpState state {};
        state.failOp = toVulkanStencilOp(opState.failOp);
        state.passOp = toVulkanStencilOp(opState.passOp);
        state.depthFailOp = toVulkanStencilOp(opState.depthFailOp);
        state.compareOp = toVulkanCompareOp(opState.compareOp);
        state.compareMask = opState.mask;
        state.reference = opState.reference;
        state.writeMask = opState.mask;
        return state;
    }

    [[nodiscard]] constexpr auto toVulkanBlendFactor(
        Pipeline::ColorBlendAttachment::BlendFactor factor) -> vk::BlendFactor
    {
        switch (factor)
        {
            case Pipeline::ColorBlendAttachment::BlendFactor::eZero:
                return vk::BlendFactor::eZero;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOne:
                return vk::BlendFactor::eOne;
            case Pipeline::ColorBlendAttachment::BlendFactor::eSrcColor:
                return vk::BlendFactor::eSrcColor;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusSrcColor:
                return vk::BlendFactor::eOneMinusSrcColor;
            case Pipeline::ColorBlendAttachment::BlendFactor::eDstColor:
                return vk::BlendFactor::eDstColor;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusDstColor:
                return vk::BlendFactor::eOneMinusDstColor;
            case Pipeline::ColorBlendAttachment::BlendFactor::eSrcAlpha:
                return vk::BlendFactor::eSrcAlpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusSrcAlpha:
                return vk::BlendFactor::eOneMinusSrcAlpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eDstAlpha:
                return vk::BlendFactor::eDstAlpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusDstAlpha:
                return vk::BlendFactor::eOneMinusDstAlpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eConstantColor:
                return vk::BlendFactor::eConstantColor;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusConstantColor:
                return vk::BlendFactor::eOneMinusConstantColor;
            case Pipeline::ColorBlendAttachment::BlendFactor::eConstantAlpha:
                return vk::BlendFactor::eConstantAlpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusConstantAlpha:
                return vk::BlendFactor::eOneMinusConstantAlpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eSrcAlphaSaturate:
                return vk::BlendFactor::eSrcAlphaSaturate;
            case Pipeline::ColorBlendAttachment::BlendFactor::eSrc1Color:
                return vk::BlendFactor::eSrc1Color;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusSrc1Color:
                return vk::BlendFactor::eOneMinusSrc1Color;
            case Pipeline::ColorBlendAttachment::BlendFactor::eSrc1Alpha:
                return vk::BlendFactor::eSrc1Alpha;
            case Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusSrc1Alpha:
                return vk::BlendFactor::eOneMinusSrc1Alpha;
        }
        return vk::BlendFactor::eZero;
    }

    [[nodiscard]] constexpr auto toVulkanBlendOp(Pipeline::ColorBlendAttachment::BlendOperation op)
        -> vk::BlendOp
    {
        switch (op)
        {
            case Pipeline::ColorBlendAttachment::BlendOperation::eAdd:
                return vk::BlendOp::eAdd;
            case Pipeline::ColorBlendAttachment::BlendOperation::eSubtract:
                return vk::BlendOp::eSubtract;
            case Pipeline::ColorBlendAttachment::BlendOperation::eReverseSubtract:
                return vk::BlendOp::eReverseSubtract;
            case Pipeline::ColorBlendAttachment::BlendOperation::eMin:
                return vk::BlendOp::eMin;
            case Pipeline::ColorBlendAttachment::BlendOperation::eMax:
                return vk::BlendOp::eMax;
        }
        return vk::BlendOp::eAdd;
    }

    [[nodiscard]] constexpr auto vulkanFormatToChannelsAndBits(vk::Format format) noexcept
        -> std::pair<uint8_t, uint8_t>  // channels, bits
    {
        switch (format)
        {
            case vk ::Format ::eR8Unorm:
                return {1, 8};
            case vk ::Format ::eR16Unorm:
                return {1, 16};
            case vk ::Format ::eR8G8Unorm:
                return {2, 8};
            case vk ::Format ::eR16G16Unorm:
                return {2, 16};
            case vk ::Format ::eR8G8B8A8Unorm:
                return {4, 8};
            case vk ::Format ::eR16G16B16A16Unorm:
                return {4, 16};
            case vk ::Format ::eR16Sfloat:
                return {1, 16};
            case vk ::Format ::eR16G16Sfloat:
                return {2, 16};
            case vk ::Format ::eR16G16B16A16Sfloat:
                return {4, 16};
            case vk ::Format ::eR32Sfloat:
                return {1, 32};
            case vk ::Format ::eR32G32Sfloat:
                return {2, 32};
            case vk ::Format ::eR32G32B32A32Sfloat:
                return {4, 32};
            case vk ::Format ::eB8G8R8A8Unorm:
                return {4, 8};
            default:
                break;
        }

        return {0, 0};
    }

}  // namespace exage::Graphics
