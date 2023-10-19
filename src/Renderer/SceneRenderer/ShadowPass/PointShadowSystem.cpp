#include <execution>
#include <limits>
#include <vector>

#include "exage/Renderer/SceneRenderer/ShadowPass/PointShadowSystem.h"

#include <fmt/format.h>

#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Pipeline.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Utils/Frustum.h"
#include "exage/Renderer/Utils/Perspective.h"

namespace exage::Renderer
{
    constexpr std::string_view VERTEX_SHADER_PATH = "shadow_pass/point_mesh.vert";
    constexpr std::string_view GEOMETRY_SHADER_PATH = "shadow_pass/point_mesh.geom";
    constexpr std::string_view FRAGMENT_SHADER_PATH = "shadow_pass/point_mesh.frag";

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec3 lightPosition;
        alignas(4) uint32_t transformIndex;
    };

    PointShadowSystem::PointShadowSystem(const PointShadowSystemCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _assetCache(createInfo.assetCache)
    {
        std::filesystem::path vertexShaderPath =
            Filesystem::getEngineShaderDirectory() / VERTEX_SHADER_PATH;
        auto vertexShaderCode =
            Graphics::compileShaderToIR(vertexShaderPath, Graphics::Shader::Stage::eVertex);

        debugAssume(vertexShaderCode.has_value(), fmt::format("Failed to compile vertex shader"));

        std::filesystem::path geometryShaderPath =
            Filesystem::getEngineShaderDirectory() / GEOMETRY_SHADER_PATH;
        auto geometryShaderCode =
            Graphics::compileShaderToIR(geometryShaderPath, Graphics::Shader::Stage::eGeometry);

        debugAssume(geometryShaderCode.has_value(),
                    fmt::format("Failed to compile geometry shader"));

        std::filesystem::path fragmentShaderPath =
            Filesystem::getEngineShaderDirectory() / FRAGMENT_SHADER_PATH;
        auto fragmentShaderCode =
            Graphics::compileShaderToIR(fragmentShaderPath, Graphics::Shader::Stage::eFragment);

        debugAssume(fragmentShaderCode.has_value(),
                    fmt::format("Failed to compile fragment shader"));

        Graphics::ShaderCreateInfo shaderCreateInfo {};
        shaderCreateInfo.compileDirectory = Filesystem::getEngineShaderCacheDirectory();

        shaderCreateInfo.stage = Graphics::Shader::Stage::eVertex;
        shaderCreateInfo.irCode = *vertexShaderCode;
        auto vertexShader = _context.createShader(shaderCreateInfo);

        shaderCreateInfo.stage = Graphics::Shader::Stage::eGeometry;
        shaderCreateInfo.irCode = *geometryShaderCode;
        auto geometryShader = _context.createShader(shaderCreateInfo);

        shaderCreateInfo.stage = Graphics::Shader::Stage::eFragment;
        shaderCreateInfo.irCode = *fragmentShaderCode;
        auto fragmentShader = _context.createShader(shaderCreateInfo);

        using Graphics::Pipeline;
        Pipeline::ShaderInfo shaderInfo {};
        shaderInfo.vertexShader = vertexShader;
        shaderInfo.geometryShader = geometryShader;
        shaderInfo.fragmentShader = fragmentShader;

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
        renderInfo.depthStencilFormat = _context.getHardwareSupport().depthFormat;

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

        _pipeline = _context.createPipeline(pipelineCreateInfo);
    }

    void PointShadowSystem::render(Graphics::CommandBuffer& commandBuffer,
                                   SceneData& sceneData) noexcept
    {
        auto view = entt::basic_view {sceneData.currentTransforms}
            | entt::basic_view {sceneData.currentPointLights};

        _pointLightRenderArrayData.clear();
        _pointLightRenderArrayData.reserve(view.size_hint());

        for (auto entity : view)
        {
            auto& light = view.get<PointLight>(entity);
            auto& transform = view.get<Transform3D>(entity);

            bool shouldRender = false;
            PointLightRenderInfo* lightRenderInfo = nullptr;
            if (!sceneData.pointLightRenderInfo.contains(entity))
            {
                lightRenderInfo = &sceneData.pointLightRenderInfo.emplace(entity);
                shouldRender = light.castShadow;
            }
            else
            {
                lightRenderInfo = &sceneData.pointLightRenderInfo.get(entity);
                shouldRender = shouldLightRenderShadow(transform, light, *lightRenderInfo);
            }

            lightRenderInfo->arrayIndex = _pointLightRenderArrayData.size();
            lightRenderInfo->position = transform.globalPosition;
            lightRenderInfo->color = light.color;
            lightRenderInfo->intensity = light.intensity;
            lightRenderInfo->physicalRadius = light.physicalRadius;
            lightRenderInfo->attenuationRadius = light.attenuationRadius;
            lightRenderInfo->shadowBias = light.shadowBias;

            PointLightRenderArray::Data::ArrayItem item {};
            item.position = transform.globalPosition;
            item.color = light.color;
            item.intensity = light.intensity;
            item.physicalRadius = light.physicalRadius;
            item.attenuationRadius = light.attenuationRadius;
            item.shadowBias = light.shadowBias;
            item.shadowMapIndex = lightRenderInfo->shadowMapIndex;

            _pointLightRenderArrayData.push_back(item);

            if (!shouldRender)
            {
                if (lightRenderInfo->shadowMap)
                {
                    lightRenderInfo->shadowMap.reset();
                    lightRenderInfo->shadowMapIndex = std::numeric_limits<uint32_t>::max();
                }

                continue;
            }

            if (!lightRenderInfo->shadowMap
                || lightRenderInfo->shadowMap->getExtent().x
                    != static_cast<uint32_t>(_renderQualitySettings.shadowResolution))
            {
                Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                frameBufferCreateInfo.extent = {_renderQualitySettings.shadowResolution,
                                                _renderQualitySettings.shadowResolution};
                frameBufferCreateInfo.colorAttachments = {};
                frameBufferCreateInfo.depthAttachment = {
                    _context.getHardwareSupport().depthFormat,
                    Graphics::Texture::UsageFlags::eDepthStencilAttachment
                        | Graphics::Texture::UsageFlags::eSampled};

                lightRenderInfo->shadowMap = _context.createFrameBuffer(frameBufferCreateInfo);
                lightRenderInfo->shadowMapIndex =
                    lightRenderInfo->shadowMap->getDepthStencilTexture()
                        ->getBindlessID(Graphics::Texture::Aspect::eDepth)
                        .id;
            }

            renderShadow(commandBuffer, sceneData, transform, light, *lightRenderInfo);
        }
    }

    void PointShadowSystem::renderShadow(Graphics::CommandBuffer& commandBuffer,
                                         SceneData& sceneData,
                                         Transform3D& transform,
                                         PointLight& light,
                                         PointLightRenderInfo& info) noexcept
    {
        commandBuffer.bindPipeline(_pipeline);

        auto view = entt::basic_view {sceneData.currentTransforms}
            | entt::basic_view {sceneData.currentStaticMeshes};

        {
            std::array<glm::mat4, 6> views {};
            views[0] = glm::lookAt(transform.globalPosition,
                                   transform.globalPosition + glm::vec3 {1.0F, 0.0F, 0.0F},
                                   glm::vec3 {0.0F, -1.0F, 0.0F});
            views[1] = glm::lookAt(transform.globalPosition,
                                   transform.globalPosition + glm::vec3 {-1.0F, 0.0F, 0.0F},
                                   glm::vec3 {0.0F, -1.0F, 0.0F});

            views[2] = glm::lookAt(transform.globalPosition,
                                   transform.globalPosition + glm::vec3 {0.0F, 1.0F, 0.0F},
                                   glm::vec3 {0.0F, 0.0F, 1.0F});

            views[3] = glm::lookAt(transform.globalPosition,
                                   transform.globalPosition + glm::vec3 {0.0F, -1.0F, 0.0F},
                                   glm::vec3 {0.0F, 0.0F, -1.0F});

            views[4] = glm::lookAt(transform.globalPosition,
                                   transform.globalPosition + glm::vec3 {0.0F, 0.0F, 1.0F},
                                   glm::vec3 {0.0F, -1.0F, 0.0F});

            views[5] = glm::lookAt(transform.globalPosition,
                                   transform.globalPosition + glm::vec3 {0.0F, 0.0F, -1.0F},
                                   glm::vec3 {0.0F, -1.0F, 0.0F});
            constexpr float NEAR_PLANE = 0.05F;

            glm::mat4 projection = perspectiveProject(glm::radians(90.0F),
                                                      1.0F,
                                                      NEAR_PLANE,
                                                      light.attenuationRadius,
                                                      _context.getAPIProperties().depthZeroToOne);

            for (uint32_t i = 0; i < 6; i++)
            {
                info.viewProjections[i] = projection * views[i];
            }
        }

        auto func = [&](auto entity)
        {
            auto& meshComponent = view.get<StaticMeshComponent>(entity);
            auto& transform = view.get<Transform3D>(entity);

            auto* meshPtr = _assetCache.getMeshIfExists(meshComponent.pathHash);
            if (meshPtr == nullptr)
            {
                fmt::print("Mesh {} not found\n", meshComponent.path);
                return;
            }

            const auto& mesh = *meshPtr;

            glm::mat4 modelViewProjections[6];

            bool intersects = false;

            for (uint8_t i = 0; i < 6; i++)
            {
                modelViewProjections[i] = info.viewProjections[i] * transform.globalMatrix;
                Frustum frustum {modelViewProjections[i]};
                if (!frustum.intersects(mesh.aabb))
                {
                    continue;
                }

                intersects = true;
            }

            if (!intersects)
            {
                return;
            }

            std::scoped_lock lock {*_mutex};
            commandBuffer.bindVertexBuffer(mesh.vertexBuffer, 0);
            commandBuffer.bindIndexBuffer(mesh.indexBuffer, 0);

            for (uint8_t i = 0; i < 6; i++)
            {
                PushConstant pushConstant {};
                pushConstant.model = transform.globalMatrix;
                pushConstant.lightPosition = transform.globalPosition;

                uint32_t lodLevel = 0;  // TODO: LOD

                std::span<const std::byte> pushConstantSpan =
                    std::as_bytes(std::span {&pushConstant, 1});

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

    auto PointShadowSystem::shouldLightRenderShadow(const Transform3D& transform,
                                                    const PointLight& light,
                                                    PointLightRenderInfo& info) noexcept -> bool
    {
        // TODO: check if objects in the light's radius have moved
        return true;
    }

}  // namespace exage::Renderer
