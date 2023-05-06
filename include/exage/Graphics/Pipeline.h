#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Graphics/Texture.h"

namespace exage::Graphics
{
    struct VertexDescription
    {
        enum class Type
        {
            eFloat,
            eU32,
            eI8,
            eI32,
        };

        enum class InputRate
        {
            eVertex,
            eInstance,
        };

        uint32_t offset;
        uint32_t components;
        uint32_t stride;
        Type type;
        InputRate inputRate;
    };

    struct ResourceDescription
    {
        enum class Type
        {
            eSampledImage,
            // eUniformBuffer,
            eStorageBuffer,
            eStorageImage,
        };

        uint32_t binding;
        Type type;
    };

    class EXAGE_EXPORT Pipeline
    {
      public:
        Pipeline() noexcept = default;
        virtual ~Pipeline() = default;

        EXAGE_DEFAULT_COPY(Pipeline);
        EXAGE_DEFAULT_MOVE(Pipeline);

        [[nodiscard]] auto isBindless() const noexcept -> bool { return _bindless; }

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
            bool stencilTest = false;
            StencilOperationState stencilFront;
            StencilOperationState stencilBack;
        };

        struct ColorBlendAttachment
        {
            enum class BlendFactor
            {
                eZero,
                eOne,
                eSrcColor,
                eOneMinusSrcColor,
                eDstColor,
                eOneMinusDstColor,
                eSrcAlpha,
                eOneMinusSrcAlpha,
                eDstAlpha,
                eOneMinusDstAlpha,
                eConstantColor,
                eOneMinusConstantColor,
                eConstantAlpha,
                eOneMinusConstantAlpha,
                eSrcAlphaSaturate,
                eSrc1Color,
                eOneMinusSrc1Color,
                eSrc1Alpha,
                eOneMinusSrc1Alpha
            };

            bool blend = false;
            BlendFactor srcColorBlendFactor = BlendFactor::eOne;
            BlendFactor dstColorBlendFactor = BlendFactor::eOne;
            BlendFactor srcAlphaBlendFactor = BlendFactor::eOne;
            BlendFactor dstAlphaBlendFactor = BlendFactor::eOne;

            enum class BlendOperation
            {
                eAdd,
                eSubtract,
                eReverseSubtract,
                eMin,
                eMax
            };
            BlendOperation colorBlendOp = BlendOperation::eAdd;
            BlendOperation alphaBlendOp = BlendOperation::eAdd;
        };

        struct ColorBlendState
        {
            std::vector<ColorBlendAttachment> attachments;
            glm::vec4 blendConstants = glm::vec4(0.f);
        };

        struct RasterState
        {
            PolygonMode polygonMode = PolygonMode::eFill;
            CullMode cullMode = CullMode::eBack;
            FrontFace frontFace = FrontFace::eCounterClockwise;

            float lineWidth = 1.f;
        };

        struct ShaderInfo
        {
            std::shared_ptr<Shader> vertexShader;
            std::shared_ptr<Shader> tessellationControlShader;
            std::shared_ptr<Shader> tessellationEvaluationShader;
            std::shared_ptr<Shader> fragmentShader;
        };

        struct RenderInfo
        {
            std::vector<Format> colorFormats;
            Format depthStencilFormat;
        };

      protected:
        explicit Pipeline(bool bindless) noexcept
            : _bindless(bindless)
        {
        }

        bool _bindless = false;
    };

    struct PipelineCreateInfo
    {
        std::vector<VertexDescription> vertexDescriptions;
        std::vector<ResourceDescription> resourceDescriptions;
        Pipeline::ShaderInfo shaderInfo;
        Pipeline::ColorBlendState colorBlendState;
        Pipeline::RenderInfo renderInfo;
        Pipeline::RasterState rasterState;
        Pipeline::DepthStencilState depthStencilState;
        uint32_t pushConstantSize;

        bool bindless = false;  // If set, resourceDescriptions are ignored
    };
}  // namespace exage::Graphics
