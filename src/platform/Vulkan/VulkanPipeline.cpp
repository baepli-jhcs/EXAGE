#include "exage/platform/Vulkan/VulkanPipeline.h"

#include "exage/platform/Vulkan/VulkanShader.h"

namespace exage::Graphics
{
    VulkanPipeline::VulkanPipeline(VulkanContext& context,
                                   const PipelineCreateInfo& createInfo) noexcept
        : _context(context)
    {
        if (createInfo.resourceManager)
        {
            _resourceManager =
                std::static_pointer_cast<VulkanResourceManager>(createInfo.resourceManager);
        }

        auto device = _context.get().getDevice();

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(4);

        if (createInfo.shaderInfo.vertexShader)
        {
            auto* vertexShader = createInfo.shaderInfo.vertexShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo createInfo;
            createInfo.module = vertexShader->getShaderModule();
            createInfo.stage = vk::ShaderStageFlagBits::eVertex;
            createInfo.pName = "main";
            shaderStages.push_back(createInfo);
        }

        if (createInfo.shaderInfo.tessellationControlShader)
        {
            auto* tessellationControlShader =
                createInfo.shaderInfo.tessellationControlShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo createInfo;
            createInfo.module = tessellationControlShader->getShaderModule();
            createInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
            createInfo.pName = "main";
            shaderStages.push_back(createInfo);
        }

        if (createInfo.shaderInfo.tessellationEvaluationShader)
        {
            auto* tessellationEvaluationShader =
                createInfo.shaderInfo.tessellationEvaluationShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo createInfo;
            createInfo.module = tessellationEvaluationShader->getShaderModule();
            createInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
            createInfo.pName = "main";
            shaderStages.push_back(createInfo);
        }

        if (createInfo.shaderInfo.fragmentShader)
        {
            auto* fragmentShader = createInfo.shaderInfo.fragmentShader->as<VulkanShader>();
            vk::PipelineShaderStageCreateInfo createInfo;
            createInfo.module = fragmentShader->getShaderModule();
            createInfo.stage = vk::ShaderStageFlagBits::eFragment;
            createInfo.pName = "main";
            shaderStages.push_back(createInfo);
        }

        std::vector<vk::VertexInputBindingDescription> vertexBindings;
        std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
        for (const auto& vertexDescription : createInfo.vertexDescriptions)
        {
            vk::VertexInputBindingDescription bindingDescription;
            bindingDescription.binding = vertexDescription.index;
            bindingDescription.stride = vertexDescription.stride;
            bindingDescription.inputRate = toVulkanVertexInputRate(vertexDescription.inputRate);
            vertexBindings.push_back(bindingDescription);

            vk::VertexInputAttributeDescription attributeDescription;
            attributeDescription.binding = vertexDescription.index;
            attributeDescription.location = vertexDescription.index;
            attributeDescription.format =
                toVulkanFormat(vertexDescription.components, vertexDescription.type);
            attributeDescription.offset = vertexDescription.offset;
            vertexAttributes.push_back(attributeDescription);
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexBindingDescriptionCount =
            static_cast<uint32_t>(vertexBindings.size());
        vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
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

        std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        VulkanContext::PipelineLayoutInfo pipelineLayoutInfo;
        pipelineLayoutInfo.resourceDescriptions = createInfo.resourceDescriptions;
        pipelineLayoutInfo.pushConstantSize = createInfo.pushConstantSize;
        pipelineLayoutInfo.resourceManager = _resourceManager.get();

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
        pipelineInfo.pDynamicState = nullptr;
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
