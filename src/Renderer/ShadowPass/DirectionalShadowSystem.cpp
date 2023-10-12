#include <execution>
#include <vector>

#include "exage/Renderer/ShadowPass/DirectionalShadowSystem.h"

#include <fmt/format.h>

#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Renderer/Locations.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Utils/Frustum.h"

namespace exage::Renderer
{
    constexpr std::string_view VERTEX_SHADER_PATH = "shadow_pass/directional_mesh.vert";

    struct PushConstant
    {
        glm::mat4 modelViewProjection;
        int32_t layer = 0;
    };

    DirectionalShadowSystem::DirectionalShadowSystem(
        const DirectionalShadowSystemCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _sceneBuffer(createInfo.sceneBuffer)
        , _assetCache(createInfo.assetCache)
        , _cascadeLevels(createInfo.cascadeLevels)
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
        vertexDescription.stride = sizeof(StaticMeshVertex);

        Graphics::VertexAttribute positionVertexDescription {};
        positionVertexDescription.offset = offsetof(StaticMeshVertex, position);
        positionVertexDescription.components = 3;
        positionVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute normalVertexDescription {};
        normalVertexDescription.offset = offsetof(StaticMeshVertex, normal);
        normalVertexDescription.components = 3;
        normalVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute uvVertexDescription {};
        uvVertexDescription.offset = offsetof(StaticMeshVertex, uv);
        uvVertexDescription.components = 2;
        uvVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute tangentVertexDescription {};
        tangentVertexDescription.offset = offsetof(StaticMeshVertex, tangent);
        tangentVertexDescription.components = 3;
        tangentVertexDescription.type = Graphics::VertexAttribute::Type::eFloat;

        Graphics::VertexAttribute bitangentVertexDescription {};
        bitangentVertexDescription.offset = offsetof(StaticMeshVertex, bitangent);
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

    void DirectionalShadowSystem::render(Graphics::CommandBuffer& commandBuffer, Scene& scene)
    {
        auto& reg = scene.registry();

        auto view = entt::basic_view {reg.storage<DirectionalLightRenderInfo>(
                        CURRENT_DIRECTIONAL_LIGHT_RENDER_INFO)}
            | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

        auto& cameraRenderInfo = getCameraRenderInfo(scene);
        auto& camera = getCameraComponent(scene);

        for (auto entity : view)
        {
            auto& lightRenderInfo = view.get<DirectionalLightRenderInfo>(entity);
            auto& transform = view.get<Transform3D>(entity);

            if (!lightRenderInfo.shadowMap)
            {
                continue;
            }

            auto texture = lightRenderInfo.shadowMap->getDepthStencilTexture();

            commandBuffer.textureBarrier(texture,
                                         Graphics::Texture::Layout::eUndefined,
                                         Graphics::Texture::Layout::eDepthStencilAttachment,
                                         Graphics::PipelineStageFlags::eTopOfPipe,
                                         Graphics::PipelineStageFlags::eEarlyFragmentTests,
                                         Graphics::Access {},
                                         Graphics::AccessFlags::eDepthStencilAttachmentWrite);

            Graphics::ClearDepthStencil clearDepthStencil {};
            clearDepthStencil.depth = 1.0F;
            clearDepthStencil.clear = true;
            clearDepthStencil.stencil = 0;
            commandBuffer.beginRendering(lightRenderInfo.shadowMap, {}, clearDepthStencil);

            glm::uvec3 extent = texture->getExtent();
            commandBuffer.setViewport({0, 0}, {extent.x, extent.y});
            commandBuffer.setScissor({0, 0}, {extent.x, extent.y});

            renderShadow(commandBuffer, scene, camera, cameraRenderInfo, lightRenderInfo);

            commandBuffer.endRendering();

            commandBuffer.textureBarrier(texture,
                                         Graphics::Texture::Layout::eDepthStencilAttachment,
                                         Graphics::Texture::Layout::eDepthStencilReadOnly,
                                         Graphics::PipelineStageFlags::eLateFragmentTests,
                                         Graphics::PipelineStageFlags::eFragmentShader,
                                         Graphics::AccessFlags::eDepthStencilAttachmentWrite,
                                         Graphics::AccessFlags::eShaderRead);
        }
    }

    void DirectionalShadowSystem::renderShadow(Graphics::CommandBuffer& commandBuffer,
                                               Scene& scene,
                                               Camera& camera,
                                               CameraRenderInfo& cameraRenderInfo,
                                               DirectionalLightRenderInfo& lightRenderInfo) noexcept
    {
        commandBuffer.bindPipeline(_pipeline);

        auto& reg = scene.registry();

        auto view = entt::basic_view {reg.storage<StaticMeshComponent>(CURRENT_MESH_COMPONENT)}
            | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

        auto func = [&](auto entity)
        {
            auto& meshComponent = view.get<StaticMeshComponent>(entity);
            auto& transform = view.get<Transform3D>(entity);

            GPUStaticMesh* meshPtr = _assetCache.get().getMeshIfExists(meshComponent.pathHash);
            if (meshPtr == nullptr)
            {
                fmt::print("Mesh {} not found\n", meshComponent.path);
                return;
            }

            const auto& mesh = *meshPtr;

            for (uint8_t i = 0; i < static_cast<uint8_t>(_cascadeLevels); i++)
            {
                glm::mat4 modelViewProjection =
                    lightRenderInfo.cascadeViewProjections[i] * transform.matrix;

                Frustum frustum {modelViewProjection};
                if (!frustum.intersects(mesh.aabb))
                {
                    return;
                }

                PushConstant pushConstant {};
                pushConstant.modelViewProjection = modelViewProjection;
                pushConstant.layer = i;

                uint32_t lodLevel = 0;  // TODO: LOD

                std::span<const std::byte> pushConstantSpan =
                    std::as_bytes(std::span {&pushConstant, 1});

                std::scoped_lock lock {*_mutex};

                commandBuffer.bindVertexBuffer(mesh.vertexBuffer, 0);
                commandBuffer.bindIndexBuffer(mesh.indexBuffer, 0);
                commandBuffer.setPushConstant(pushConstantSpan);
                commandBuffer.drawIndexed(mesh.lods[lodLevel].indexCount,
                                          mesh.lods[lodLevel].indexOffset,
                                          mesh.lods[lodLevel].vertexOffset,
                                          1,
                                          0);
            }
        };

        std::for_each(std::execution::par, view.begin(), view.end(), func);
    }

}  // namespace exage::Renderer
