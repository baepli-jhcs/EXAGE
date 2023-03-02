#pragma once

#include <variant>

#include "Texture.h"
#include "Core/Core.h"

namespace exage::Graphics
{
    BEGIN_RAW_BITFLAGS(PipelineStage)
        RAW_FLAG(eTopOfPipe)
        RAW_FLAG(eDrawIndirect)
        RAW_FLAG(eVertexInput)
        RAW_FLAG(eVertexShader)
        RAW_FLAG(eTessellationControlShader)
        RAW_FLAG(eTessellationEvaluationShader)
        RAW_FLAG(eFragmentShader)
        RAW_FLAG(eEarlyFragmentTests)
        RAW_FLAG(eLateFragmentTests)
        RAW_FLAG(eColorAttachmentOutput)
        RAW_FLAG(eComputeShader)
        RAW_FLAG(eTransfer)
        RAW_FLAG(eBottomOfPipe)
        RAW_FLAG(eHost)
        RAW_FLAG(eAllGraphics)
        RAW_FLAG(eAllCommands)
    END_RAW_BITFLAGS(PipelineStage)

    BEGIN_RAW_BITFLAGS(Access)
        RAW_FLAG(eIndirectCommandRead)
        RAW_FLAG(eIndexRead)
        RAW_FLAG(eVertexAttributeRead)
        RAW_FLAG(eUniformRead)
        RAW_FLAG(eInputAttachmentRead)
        RAW_FLAG(eShaderRead)
        RAW_FLAG(eShaderWrite)
        RAW_FLAG(eColorAttachmentRead)
        RAW_FLAG(eColorAttachmentWrite)
        RAW_FLAG(eDepthStencilAttachmentRead)
        RAW_FLAG(eDepthStencilAttachmentWrite)
        RAW_FLAG(eTransferRead)
        RAW_FLAG(eTransferWrite)
        RAW_FLAG(eHostRead)
        RAW_FLAG(eHostWrite)
        RAW_FLAG(eMemoryRead)
        RAW_FLAG(eMemoryWrite)
    END_RAW_BITFLAGS(Access)


    struct DrawCommand
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };

    struct DrawIndexedCommand
    {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        uint32_t firstInstance;
    };

    struct TextureBarrier
    {
        Texture& texture;
        Texture::Layout newLayout;
        PipelineStage srcStage;
        PipelineStage dstStage;
        Access srcAccess;
        Access dstAccess;
    };

    using GPUCommand = std::variant<DrawCommand, DrawIndexedCommand, TextureBarrier>;
}
