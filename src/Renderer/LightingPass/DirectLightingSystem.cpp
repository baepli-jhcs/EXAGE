#include <filesystem>

#include "exage/Renderer/LightingPass/DirectLightingSystem.h"

#include <fmt/format.h>

#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Utils/Buffer.h"

namespace exage::Renderer
{
    constexpr std::string_view VERTEX_SHADER_PATH = "general/fullscreen.vert";
    constexpr std::string_view FRAGMENT_SHADER_PATH = "lighting_pass/direct.frag";

    struct PushConstant
    {
        alignas(4) uint32_t pointLightBufferIndex;
        alignas(4) uint32_t directionalLightBufferIndex;
        alignas(4) uint32_t spotLightBufferIndex;
        alignas(4) uint32_t positionTextureIndex;
        alignas(4) uint32_t normalTextureIndex;
        alignas(4) uint32_t albedoTextureIndex;
        alignas(4) uint32_t metallicTextureIndex;
        alignas(4) uint32_t roughnessTextureIndex;
        alignas(4) uint32_t occlusionTextureIndex;
        alignas(4) uint32_t emissiveTextureIndex;
        alignas(4) uint32_t cameraBufferIndex;
        alignas(4) uint32_t samplerIndex;
    };

    DirectLightingSystem::DirectLightingSystem(
        const DirectLightingSystemCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
    {
        std::filesystem::path vertexShaderPath =
            Filesystem::getEngineShaderDirectory() / VERTEX_SHADER_PATH;
        auto vertexShaderCode =
            Graphics::compileShaderToIR(vertexShaderPath, Graphics::Shader::Stage::eVertex);

        debugAssume(vertexShaderCode.has_value(), fmt::format("Failed to compile vertex shader"));

        std::filesystem::path fragmentShaderPath =
            Filesystem::getEngineShaderDirectory() / FRAGMENT_SHADER_PATH;
        auto fragmentShaderCode =
            Graphics::compileShaderToIR(fragmentShaderPath, Graphics::Shader::Stage::eFragment);

        debugAssume(fragmentShaderCode.has_value(), "Failed to compile fragment shader");

        Graphics::ShaderCreateInfo shaderCreateInfo {};
        shaderCreateInfo.compileDirectory = Filesystem::getEngineShaderCacheDirectory();

        shaderCreateInfo.stage = Graphics::Shader::Stage::eVertex;
        shaderCreateInfo.irCode = *vertexShaderCode;
        auto vertexShader = _context.get().createShader(shaderCreateInfo);

        shaderCreateInfo.stage = Graphics::Shader::Stage::eFragment;
        shaderCreateInfo.irCode = *fragmentShaderCode;
        auto fragmentShader = _context.get().createShader(shaderCreateInfo);

        using Graphics::Pipeline;
        Pipeline::ShaderInfo shaderInfo {};
        shaderInfo.vertexShader = vertexShader;
        shaderInfo.fragmentShader = fragmentShader;

        Pipeline::ColorBlendState colorBlendState {};
        colorBlendState.blendConstants = {0.0F, 0.0F, 0.0F, 0.0F};
        colorBlendState.attachments.resize(1);
        colorBlendState.attachments[0].blend = false;

        Pipeline::DepthStencilState depthStencilState {};
        depthStencilState.depthTest = false;
        depthStencilState.writeDepth = false;

        Pipeline::RenderInfo renderInfo {};
        renderInfo.colorFormats = {Graphics::Format::eRGBA16f};

        Pipeline::RasterState rasterState {};
        rasterState.cullMode = Pipeline::CullMode::eNone;
        rasterState.frontFace = Pipeline::FrontFace::eCounterClockwise;
        rasterState.polygonMode = Pipeline::PolygonMode::eFill;
        rasterState.lineWidth = 1.0F;

        Graphics::PipelineCreateInfo pipelineCreateInfo {};
        pipelineCreateInfo.vertexDescription = {};
        pipelineCreateInfo.shaderInfo = shaderInfo;
        pipelineCreateInfo.colorBlendState = colorBlendState;
        pipelineCreateInfo.renderInfo = renderInfo;
        pipelineCreateInfo.rasterState = rasterState;
        pipelineCreateInfo.depthStencilState = depthStencilState;
        pipelineCreateInfo.pushConstantSize = sizeof(PushConstant);
        pipelineCreateInfo.bindless = true;

        _pipeline = _context.get().createPipeline(pipelineCreateInfo);

        Graphics::SamplerCreateInfo samplerCreateInfo {};
        _sampler = _context.get().createSampler(samplerCreateInfo);
    }

    void DirectLightingSystem::render(Graphics::CommandBuffer& commandBuffer,
                                      Scene& scene,
                                      const DirectLightingSystemRenderInfo& renderInfo) noexcept
    {
        auto& directionalLightArray = getDirectionalLightRenderArray(scene);
        auto& pointLightArray = getPointLightRenderArray(scene);
        auto& spotLightArray = getSpotLightRenderArray(scene);

        PushConstant pushConstant {};
        pushConstant.pointLightBufferIndex = pointLightArray.buffer->currentBindlessID().id;
        pushConstant.directionalLightBufferIndex =
            directionalLightArray.buffer->currentBindlessID().id;
        pushConstant.spotLightBufferIndex = spotLightArray.buffer->currentBindlessID().id;
        pushConstant.positionTextureIndex = renderInfo.position->getBindlessID().id;
        pushConstant.normalTextureIndex = renderInfo.normal->getBindlessID().id;
        pushConstant.albedoTextureIndex = renderInfo.albedo->getBindlessID().id;
        pushConstant.metallicTextureIndex = renderInfo.metallic->getBindlessID().id;
        pushConstant.roughnessTextureIndex = renderInfo.roughness->getBindlessID().id;
        pushConstant.occlusionTextureIndex = renderInfo.occlusion->getBindlessID().id;
        pushConstant.emissiveTextureIndex = renderInfo.emissive->getBindlessID().id;

        const auto& cameraRenderInfo = getCameraRenderInfo(scene);
        pushConstant.cameraBufferIndex = cameraRenderInfo.buffer->currentBindlessID().id;

        pushConstant.samplerIndex = _sampler->getBindlessID().id;

        commandBuffer.bindPipeline(_pipeline);
        commandBuffer.setViewport({0.F, 0.F}, renderInfo.extent);
        commandBuffer.setScissor({0, 0}, renderInfo.extent);
        commandBuffer.setPushConstant(pushConstant);

        commandBuffer.draw(3, 0, 1, 0);
    }

}  // namespace exage::Renderer
