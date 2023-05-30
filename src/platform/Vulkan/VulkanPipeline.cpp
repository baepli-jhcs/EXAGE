#include "exage/platform/Vulkan/VulkanPipeline.h"

#include "exage/platform/Vulkan/VulkanShader.h"
#include "vulkan/vulkan_structs.hpp"

namespace exage::Graphics
{
    VulkanPipeline::VulkanPipeline(VulkanContext& context,
                                   const PipelineCreateInfo& createInfo) noexcept
        : Pipeline(createInfo.bindless)
        , _context(context)
    {
        auto device = _context.get().getDevice();

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(4);

        if (createInfo.shaderInfo.vertexShader)
        {
            auto* vertexShader = createInfo.shaderInfo.vertexShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.module = vertexShader->getShaderModule();
            shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
            shaderStageCreateInfo.pName = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (createInfo.shaderInfo.tessellationControlShader)
        {
            auto* tessellationControlShader =
                createInfo.shaderInfo.tessellationControlShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.module = tessellationControlShader->getShaderModule();
            shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
            shaderStageCreateInfo.pName = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (createInfo.shaderInfo.tessellationEvaluationShader)
        {
            auto* tessellationEvaluationShader =
                createInfo.shaderInfo.tessellationEvaluationShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.module = tessellationEvaluationShader->getShaderModule();
            shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
            shaderStageCreateInfo.pName = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (createInfo.shaderInfo.fragmentShader)
        {
            auto* fragmentShader = createInfo.shaderInfo.fragmentShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.module = fragmentShader->getShaderModule();
            shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
            shaderStageCreateInfo.pName = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        vk::VertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = createInfo.vertexDescription.stride;
        bindingDescription.inputRate =
            toVulkanVertexInputRate(createInfo.vertexDescription.inputRate);

        std::vector<vk::VertexInputAttributeDescription> vertexAttributes;

        for (uint32_t index = 0; const auto& attribute : createInfo.vertexDescription.attributes)
        {
            vk::VertexInputAttributeDescription attributeDescription;
            attributeDescription.binding = 0;
            attributeDescription.location = index;
            attributeDescription.format = toVulkanFormat(attribute.components, attribute.type);
            attributeDescription.offset = attribute.offset;
            vertexAttributes.push_back(attributeDescription);

            ++index;
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(vertexAttributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        vk::PipelineTessellationStateCreateInfo tessellationState;
        tessellationState.patchControlPoints = 3;

        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = toVulkanPolygonMode(createInfo.rasterState.polygonMode);
        rasterizer.lineWidth = createInfo.rasterState.lineWidth;
        rasterizer.cullMode = toVulkanCullModeFlags(createInfo.rasterState.cullMode);
        rasterizer.frontFace = toVulkanFrontFace(createInfo.rasterState.frontFace);
        rasterizer.depthBiasEnable = VK_FALSE;

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineDepthStencilStateCreateInfo depthStencil;
        depthStencil.depthTestEnable = createInfo.depthStencilState.depthTest;
        depthStencil.depthWriteEnable = createInfo.depthStencilState.writeDepth;
        depthStencil.depthCompareOp = toVulkanCompareOp(createInfo.depthStencilState.depthCompare);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = createInfo.depthStencilState.stencilTest;
        depthStencil.front = toVulkanStencilOpState(createInfo.depthStencilState.stencilFront);
        depthStencil.back = toVulkanStencilOpState(createInfo.depthStencilState.stencilBack);

        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
        colorBlendAttachments.reserve(createInfo.colorBlendState.attachments.size());

        debugAssert(createInfo.colorBlendState.attachments.size()
                        == createInfo.renderInfo.colorFormats.size(),
                    "Color blend state attachments must match render info color formats");

        for (const auto& attachment : createInfo.colorBlendState.attachments)
        {
            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR
                | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
                | vk::ColorComponentFlagBits::eA;
            colorBlendAttachment.blendEnable = attachment.blend;
            colorBlendAttachment.srcColorBlendFactor =
                toVulkanBlendFactor(attachment.srcColorBlendFactor);
            colorBlendAttachment.dstColorBlendFactor =
                toVulkanBlendFactor(attachment.dstColorBlendFactor);
            colorBlendAttachment.srcAlphaBlendFactor =
                toVulkanBlendFactor(attachment.srcAlphaBlendFactor);
            colorBlendAttachment.dstAlphaBlendFactor =
                toVulkanBlendFactor(attachment.dstAlphaBlendFactor);
            colorBlendAttachment.colorBlendOp = toVulkanBlendOp(attachment.colorBlendOp);
            colorBlendAttachment.alphaBlendOp = toVulkanBlendOp(attachment.alphaBlendOp);
            colorBlendAttachments.push_back(colorBlendAttachment);
        }

        vk::PipelineColorBlendStateCreateInfo colorBlending;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();
        colorBlending.blendConstants[0] = createInfo.colorBlendState.blendConstants[0];
        colorBlending.blendConstants[1] = createInfo.colorBlendState.blendConstants[1];
        colorBlending.blendConstants[2] = createInfo.colorBlendState.blendConstants[2];
        colorBlending.blendConstants[3] = createInfo.colorBlendState.blendConstants[3];

        VulkanContext::PipelineLayoutInfo pipelineLayoutInfo;

        if (createInfo.bindless)
        {
            pipelineLayoutInfo.bindless = createInfo.bindless;
        }
        else
        {
            pipelineLayoutInfo.resourceDescriptions = createInfo.resourceDescriptions;
        }

        pipelineLayoutInfo.pushConstantSize = createInfo.pushConstantSize;

        _pipelineLayout = _context.get().getOrCreatePipelineLayout(pipelineLayoutInfo);

        vk::PipelineRenderingCreateInfo renderingInfo {};

        std::vector<vk::Format> colorAttachments {};
        colorAttachments.reserve(createInfo.renderInfo.colorFormats.size());
        for (const auto& format : createInfo.renderInfo.colorFormats)
        {
            colorAttachments.push_back(toVulkanFormat(format));
        }

        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
        renderingInfo.pColorAttachmentFormats = colorAttachments.data();
        renderingInfo.depthAttachmentFormat =
            toVulkanFormat(createInfo.renderInfo.depthStencilFormat);
        renderingInfo.stencilAttachmentFormat = renderingInfo.depthAttachmentFormat;

        std::array<vk::DynamicState, 2> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pTessellationState = &tessellationState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.setPNext(&renderingInfo);

        vk::ResultValue<vk::Pipeline> result =
            _context.get().getDevice().createGraphicsPipeline(nullptr, pipelineInfo, nullptr);
        checkVulkan(result.result);

        _pipeline = result.value;
    }

    VulkanPipeline::~VulkanPipeline()
    {
        if (_pipeline)
        {
            _context.get().getDevice().destroyPipeline(_pipeline);
        }
    }
    VulkanPipeline::VulkanPipeline(VulkanPipeline&& old) noexcept
        : _context(old._context)
        , _pipelineLayout(old._pipelineLayout)
        , _pipeline(old._pipeline)
    {
        old._pipeline = nullptr;
    }

    auto VulkanPipeline::operator=(VulkanPipeline&& old) noexcept -> VulkanPipeline&
    {
        if (this != &old)
        {
            if (_pipeline)
            {
                _context.get().getDevice().destroyPipeline(_pipeline);
            }
            _context = old._context;
            _pipelineLayout = old._pipelineLayout;
            _pipeline = old._pipeline;
            old._pipeline = nullptr;
        }
        return *this;
    }
}  // namespace exage::Graphics
