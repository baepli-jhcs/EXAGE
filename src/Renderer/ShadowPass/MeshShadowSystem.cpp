#include <execution>
#include <vector>

#include "exage/Renderer/ShadowPass/MeshShadowSystem.h"

#include <fmt/format.h>

#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Renderer/Locations.h"
#include "exage/Renderer/Scene/Light.h"

namespace exage::Renderer
{
    constexpr std::string_view VERTEX_SHADER_PATH = "shadow_pass/mesh.vert";

    struct PushConstant
    {
        glm::mat4 modelViewProjection;
    };

    MeshShadowSystem::MeshShadowSystem(const MeshShadowSystemCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _sceneBuffer(createInfo.sceneBuffer)
        , _assetCache(createInfo.assetCache)
    {
        std::filesystem::path vertexShaderPath =
            Filesystem::getEngineShaderDirectory() / VERTEX_SHADER_PATH;
        auto vertexShaderCode =
            Graphics::compileShaderToIR(vertexShaderPath, Graphics::Shader::Stage::eVertex);

        debugAssume(vertexShaderCode.has_value(), fmt::format("Failed to compile vertex shader"));

        Graphics::ShaderCreateInfo shaderCreateInfo {};
        shaderCreateInfo.compileDirectory = Filesystem::getEngineShaderCacheDirectory();

        shaderCreateInfo.stage = Graphics::Shader::Stage::eVertex;
        shaderCreateInfo.irCode = *vertexShaderCode;
        auto vertexShader = _context.get().createShader(shaderCreateInfo);

        using Graphics::Pipeline;
        Pipeline::ShaderInfo shaderInfo {};
        shaderInfo.vertexShader = vertexShader;

        Graphics::VertexDescription vertexDescription {};
        vertexDescription.inputRate = Graphics::VertexDescription::InputRate::eVertex;
        vertexDescription.stride = sizeof(MeshVertex);

        Graphics::VertexAttribute positionVertexDescription {};
        positionVertexDescription.offset = offsetof(MeshVertex, position);
        positionVertexDescription.components = 3;
        positionVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute normalVertexDescription {};
        normalVertexDescription.offset = offsetof(MeshVertex, normal);
        normalVertexDescription.components = 3;
        normalVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute uvVertexDescription {};
        uvVertexDescription.offset = offsetof(MeshVertex, uv);
        uvVertexDescription.components = 2;
        uvVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute tangentVertexDescription {};
        tangentVertexDescription.offset = offsetof(MeshVertex, tangent);
        tangentVertexDescription.components = 3;
        tangentVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute bitangentVertexDescription {};
        bitangentVertexDescription.offset = offsetof(MeshVertex, bitangent);
        bitangentVertexDescription.components = 3;
        bitangentVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        vertexDescription.attributes = {positionVertexDescription,
                                        normalVertexDescription,
                                        uvVertexDescription,
                                        tangentVertexDescription,
                                        bitangentVertexDescription};

        Pipeline::RenderInfo renderInfo {};
        renderInfo.depthStencilFormat = _context.get().getHardwareSupport().depthFormat;

        Pipeline::RasterState rasterState {};
        rasterState.cullMode = Pipeline::CullMode::eNone;  // TODO: Culling
        rasterState.frontFace = Pipeline::FrontFace::eCounterClockwise;
        rasterState.polygonMode = Pipeline::PolygonMode::eFill;
        rasterState.lineWidth = 1.0F;

        Pipeline::DepthStencilState depthStencilState {};
        depthStencilState.depthTest = true;
        depthStencilState.writeDepth = true;
        depthStencilState.depthCompare = Pipeline::CompareOperation::eLessOrEqual;
        depthStencilState.stencilTest = false;

        Graphics::PipelineCreateInfo pipelineCreateInfo {};
        pipelineCreateInfo.vertexDescription = vertexDescription;
        pipelineCreateInfo.shaderInfo = shaderInfo;
        pipelineCreateInfo.renderInfo = renderInfo;
        pipelineCreateInfo.rasterState = rasterState;
        pipelineCreateInfo.depthStencilState = depthStencilState;
        pipelineCreateInfo.pushConstantSize = sizeof(PushConstant);
        pipelineCreateInfo.bindless = true;

        _pipeline = _context.get().createPipeline(pipelineCreateInfo);
    }

    void MeshShadowSystem::render(Graphics::CommandBuffer& commandBuffer, Scene& scene)
    {
        auto& reg = scene.registry();

        auto pointLightView =
            entt::basic_view {reg.storage<PointLightRenderInfo>(CURRENT_POINT_LIGHT_RENDER_INFO)};

        for (auto entity : pointLightView)
        {
            auto& pointLightRenderInfo = pointLightView.get<PointLightRenderInfo>(entity);
            auto texture = pointLightRenderInfo.shadowMap->getDepthStencilTexture();

            commandBuffer.textureBarrier(texture,
                                         Graphics::Texture::Layout::eDepthStencilAttachment,
                                         Graphics::PipelineStageFlags::eTopOfPipe,
                                         Graphics::PipelineStageFlags::eFragmentShader,
                                         Graphics::Access {},
                                         Graphics::AccessFlags::eShaderRead);

            Graphics::ClearDepthStencil clearDepthStencil {};
            clearDepthStencil.depth = 1.0F;
            clearDepthStencil.clear = true;
            clearDepthStencil.stencil = 0;
            commandBuffer.beginRendering(pointLightRenderInfo.shadowMap, {}, clearDepthStencil);

            commandBuffer.endRendering();
        }
    }

}  // namespace exage::Renderer
