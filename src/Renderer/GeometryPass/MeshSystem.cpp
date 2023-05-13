#include <execution>
#include <vector>

#include "exage/Renderer/GeometryPass/MeshSystem.h"

#include <stdint.h>

#include "exage/Core/Debug.h"
#include "exage/Filesystem/Directories.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Locations.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/Transform.h"
#include "fmt/core.h"

namespace exage::Renderer
{
    auto aabbInFrustum(const AABB& aabb, const glm::mat4& modelViewProjection) noexcept -> bool
    {
        glm::vec4 corners[8] = {
            {aabb.min.x, aabb.min.y, aabb.min.z, 1.0},  // x y z
            {aabb.max.x, aabb.min.y, aabb.min.z, 1.0},  // X y z
            {aabb.min.x, aabb.max.y, aabb.min.z, 1.0},  // x Y z
            {aabb.max.x, aabb.max.y, aabb.min.z, 1.0},  // X Y z

            {aabb.min.x, aabb.min.y, aabb.max.z, 1.0},  // x y Z
            {aabb.max.x, aabb.min.y, aabb.max.z, 1.0},  // X y Z
            {aabb.min.x, aabb.max.y, aabb.max.z, 1.0},  // x Y Z
            {aabb.max.x, aabb.max.y, aabb.max.z, 1.0},  // X Y Z
        };

        bool inFrustum = false;

        for (const auto& corner : corners)
        {
            glm::vec4 transformed = modelViewProjection * corner;

            if (transformed.x >= -transformed.w && transformed.x <= transformed.w
                && transformed.y >= -transformed.w && transformed.y <= transformed.w
                && transformed.z >= -transformed.w && transformed.z <= transformed.w)
            {
                inFrustum = true;
                break;
            }
        }

        return inFrustum;
    }

    struct BindlessPushConstant
    {
        uint32_t cameraIndex;
        uint32_t transformIndex;
        uint32_t materialIndex;
    };

    constexpr std::string_view VERTEX_SHADER_PATH = "geometry_pass/mesh_system.vert";
    constexpr std::string_view FRAGMENT_SHADER_PATH = "geometry_pass/mesh_system.frag";

    MeshSystem::MeshSystem(const MeshSystemCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _sceneBuffer(createInfo.sceneBuffer)
    {
        auto vertexShaderCode =
            Graphics::compileShaderToIR(Filesystem::getEngineShaderDirectory() / VERTEX_SHADER_PATH,
                                        Graphics::Shader::Stage::eVertex);

        debugAssume(vertexShaderCode.has_value(), "Failed to compile vertex shader");

        auto fragmentShaderCode = Graphics::compileShaderToIR(
            Filesystem::getEngineShaderDirectory() / FRAGMENT_SHADER_PATH,
            Graphics::Shader::Stage::eFragment);

        debugAssume(fragmentShaderCode.has_value(), "Failed to compile fragment shader");

        Graphics::ShaderCreateInfo shaderCreateInfo {};
        shaderCreateInfo.compileDirectory = Filesystem::getEngineShaderCacheDirectory();

        shaderCreateInfo.stage = Graphics::Shader::Stage::eVertex;
        shaderCreateInfo.irCode = vertexShaderCode.value();
        auto vertexShader = _context.get().createShader(shaderCreateInfo);

        shaderCreateInfo.stage = Graphics::Shader::Stage::eFragment;
        shaderCreateInfo.irCode = fragmentShaderCode.value();
        auto fragmentShader = _context.get().createShader(shaderCreateInfo);

        using Graphics::Pipeline;
        Pipeline::ShaderInfo shaderInfo {};
        shaderInfo.vertexShader = vertexShader;
        shaderInfo.fragmentShader = fragmentShader;

        Graphics::VertexDescription positionVertexDescription {};
        positionVertexDescription.offset = offsetof(MeshVertex, position);
        positionVertexDescription.components = 3;
        positionVertexDescription.stride = sizeof(float) * 3;
        positionVertexDescription.type = Graphics::VertexDescription::Type::eFloat;
        positionVertexDescription.inputRate = Graphics::VertexDescription::InputRate::eVertex;

        Graphics::VertexDescription normalVertexDescription {};
        normalVertexDescription.offset = offsetof(MeshVertex, normal);
        normalVertexDescription.components = 3;
        normalVertexDescription.stride = sizeof(float) * 3;
        normalVertexDescription.type = Graphics::VertexDescription::Type::eFloat;
        normalVertexDescription.inputRate = Graphics::VertexDescription::InputRate::eVertex;

        Graphics::VertexDescription uvVertexDescription {};
        uvVertexDescription.offset = offsetof(MeshVertex, uv);
        uvVertexDescription.components = 2;
        uvVertexDescription.stride = sizeof(float) * 2;
        uvVertexDescription.type = Graphics::VertexDescription::Type::eFloat;
        uvVertexDescription.inputRate = Graphics::VertexDescription::InputRate::eVertex;

        Graphics::VertexDescription tangentVertexDescription {};
        tangentVertexDescription.offset = offsetof(MeshVertex, tangent);
        tangentVertexDescription.components = 3;
        tangentVertexDescription.stride = sizeof(float) * 3;
        tangentVertexDescription.type = Graphics::VertexDescription::Type::eFloat;
        tangentVertexDescription.inputRate = Graphics::VertexDescription::InputRate::eVertex;

        Graphics::VertexDescription bitangentVertexDescription {};
        bitangentVertexDescription.offset = offsetof(MeshVertex, bitangent);
        bitangentVertexDescription.components = 3;
        bitangentVertexDescription.stride = sizeof(float) * 3;
        bitangentVertexDescription.type = Graphics::VertexDescription::Type::eFloat;
        bitangentVertexDescription.inputRate = Graphics::VertexDescription::InputRate::eVertex;

        std::vector vertexDescriptions = {positionVertexDescription,
                                          normalVertexDescription,
                                          uvVertexDescription,
                                          tangentVertexDescription,
                                          bitangentVertexDescription};

        Pipeline::ColorBlendState colorBlendState {};
        colorBlendState.blendConstants = {0.0F, 0.0F, 0.0F, 0.0F};
        colorBlendState.attachments.resize(7);
        // Position, normal, albedo, metallic, roughness, occlusion, emissive
        colorBlendState.attachments[0].blend = true;
        colorBlendState.attachments[0].srcColorBlendFactor =
            Pipeline::ColorBlendAttachment::BlendFactor::eSrcAlpha;
        colorBlendState.attachments[0].dstColorBlendFactor =
            Pipeline::ColorBlendAttachment::BlendFactor::eOneMinusSrcAlpha;
        colorBlendState.attachments[0].colorBlendOp =
            Pipeline::ColorBlendAttachment::BlendOperation::eAdd;
        colorBlendState.attachments[0].srcAlphaBlendFactor =
            Pipeline::ColorBlendAttachment::BlendFactor::eOne;
        colorBlendState.attachments[0].dstAlphaBlendFactor =
            Pipeline::ColorBlendAttachment::BlendFactor::eZero;

        colorBlendState.attachments[1] = colorBlendState.attachments[0];
        colorBlendState.attachments[2] = colorBlendState.attachments[0];
        colorBlendState.attachments[3] = colorBlendState.attachments[0];
        colorBlendState.attachments[4] = colorBlendState.attachments[0];
        colorBlendState.attachments[5] = colorBlendState.attachments[0];
        colorBlendState.attachments[6] = colorBlendState.attachments[0];

        Pipeline::RenderInfo renderInfo {};
        renderInfo.colorFormats = {Graphics::Format::eRGBA16f,
                                   Graphics::Format::eRGBA16f,
                                   Graphics::Format::eRGBA16f,
                                   Graphics::Format::eR16f,
                                   Graphics::Format::eR16f,
                                   Graphics::Format::eR16f,
                                   Graphics::Format::eRGBA16f};
        renderInfo.depthStencilFormat = _context.get().getHardwareSupport().depthFormat;

        Pipeline::RasterState rasterState {};
        rasterState.cullMode = Pipeline::CullMode::eBack;
        rasterState.frontFace = Pipeline::FrontFace::eCounterClockwise;
        rasterState.polygonMode = Pipeline::PolygonMode::eFill;
        rasterState.lineWidth = 1.0F;

        Pipeline::DepthStencilState depthStencilState {};
        depthStencilState.depthTest = true;
        depthStencilState.writeDepth = true;
        depthStencilState.depthCompare = Pipeline::CompareOperation::eLessOrEqual;
        depthStencilState.stencilTest = false;

        Graphics::PipelineCreateInfo pipelineCreateInfo {};
        pipelineCreateInfo.vertexDescriptions = vertexDescriptions;
        pipelineCreateInfo.shaderInfo = shaderInfo;
        pipelineCreateInfo.colorBlendState = colorBlendState;
        pipelineCreateInfo.renderInfo = renderInfo;
        pipelineCreateInfo.rasterState = rasterState;
        pipelineCreateInfo.depthStencilState = depthStencilState;
        pipelineCreateInfo.pushConstantSize = sizeof(BindlessPushConstant);
        pipelineCreateInfo.bindless = true;

        _pipeline = _context.get().createPipeline(pipelineCreateInfo);
    }

    void MeshSystem::render(Graphics::CommandBuffer& commandBuffer, Scene& scene)
    {
        commandBuffer.bindPipeline(_pipeline);

        auto view = entt::basic_view {scene.registry().storage<GPUMesh>(CURRENT_RENDER_DATA)}
            | entt::basic_view {scene.registry().storage<TransformRenderInfo>(CURRENT_RENDER_DATA)};

        const auto& cameraStorage = scene.registry().storage<CameraRenderInfo>(CURRENT_RENDER_DATA);
        const auto cameraEntity = getSceneCamera(scene);
        const auto& cameraInfo = cameraStorage.get(cameraEntity);

        auto func = [&](Entity entity)
        {
            const auto& mesh = view.get<GPUMesh>(entity);
            const auto& transform = view.get<TransformRenderInfo>(entity);

            // Frustum culling using meshComponent.mesh.aabb
            if (!aabbInFrustum(mesh.aabb, transform.data.modelViewProjection))
            {
                return;
            }

            if (!mesh.material.buffer)
            {
                fmt::print("Mesh {} has no material\n", static_cast<uint32_t>(entity));
                return;
            }

            BindlessPushConstant pushConstant {};
            pushConstant.cameraIndex = cameraInfo.buffer->currentBindlessID().id;
            pushConstant.transformIndex = transform.buffer->currentBindlessID().id;
            pushConstant.materialIndex = mesh.material.buffer->getBindlessID().id;

            size_t lodLevel = 0;  // TODO: LOD

            commandBuffer.insertDataDependency(mesh.material.buffer);
            pushConstant.materialIndex = mesh.material.buffer->getBindlessID().id;

            commandBuffer.setPushConstant(sizeof(pushConstant),
                                          reinterpret_cast<std::byte*>(&pushConstant));

            commandBuffer.drawIndexed(mesh.lods[lodLevel].indexCount,
                                      mesh.lods[lodLevel].indexOffset,
                                      mesh.lods[lodLevel].vertexOffset,
                                      1,
                                      0);
        };

        std::for_each(std::execution::parallel_policy(), view.begin(), view.end(), func);
    }
}  // namespace exage::Renderer
