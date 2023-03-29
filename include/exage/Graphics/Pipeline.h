#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"

namespace exage::Graphics
{
    struct VertexDescription
    {
        enum class Type
        {
            eF32,
            eF64,
            eI8,
            eI16,
            eI32,
            eI64,
            eU8,
            eU16,
            eU32,
            eBool,
        };

        enum class InputRate
        {
            eVertex,
            eInstance,
        };

        uint32_t index;
        uint32_t offset;
        uint32_t components;
        uint32_t stride;
        Type type;
        InputRate inputRate;
    };

    class EXAGE_EXPORT Pipeline
    {
      public:
        Pipeline() noexcept = default;
        virtual ~Pipeline() = default;

        enum class CompareOperation;
        enum class CullMode;
        enum class FrontFace;
        enum class PolygonMode;
        struct DepthStencilState;
        struct RasterState;
        struct RenderInfo;

        EXAGE_DEFAULT_COPY(Pipeline);
        EXAGE_DEFAULT_MOVE(Pipeline);

        EXAGE_BASE_API(API, Pipeline);

        enum class CompareOperation
        {
            eLess,
            eEqual,
            eLessOrEqual,
            eGreater,
            eNotEqual,
            eGreaterOrEqual
        };

        enum class CullMode
        {
            eNone,
            eFront,
            eBack
        };

        enum class FrontFace
        {
            eCounterClockwise,
            eClockwise,
        };

        enum class PolygonMode
        {
            eFill,
            eLine,
            ePoint
        };

        struct DepthStencilState
        {
            enum class StencilOperation
            {
                eKeep,
                eZero,
                eReplace,
                eIncrementClamp,
                eDecrementClamp,
                eIncrementWrap,
                eDecrementWrap,
                eInvert
            };

            struct StencilOperationState
            {
                CompareOperation compareOp;
                StencilOperation failOp;
                StencilOperation passOp;
                StencilOperation depthFailOp;
                uint32_t mask;
                uint32_t reference;
            };

            bool depthTest = true;
            bool writeDepth = true;
            CompareOperation depthCompare;
            bool stencilTest = true;
            StencilOperationState stencilFront;
            StencilOperationState stencilBack;
        };

        struct RasterState
        {
            PolygonMode polygonMode;
            CullMode cullMode;
            FrontFace frontFace;

            float lineWidth = 1.f;
        };

        struct RenderInfo
        {
            std::vector<Texture::Format> colorFormats;
            Texture::Format depthStencilFormat;
        };
    };

}  // namespace exage::Graphics
