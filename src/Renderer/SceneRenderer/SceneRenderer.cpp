#include "exage/Renderer/SceneRenderer/SceneRenderer.h"

namespace exage::Renderer
{
    namespace
    {
        template<typename T>
        void copyComponent(const entt::registry& reg, entt::storage<T>& renderStorage)
        {
            auto view = reg.view<T>();

            renderStorage.clear();
            renderStorage.insert(view.begin(), view.end(), view.storage().begin());
        }
    }  // namespace

    SceneRenderer::SceneRenderer(const SceneRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _assetCache(createInfo.assetCache)
        , _renderQualitySettings(createInfo.renderQualitySettings)
    {
    }

    void SceneRenderer::prepareSceneData(const Scene& scene, SceneData& sceneData) noexcept
    {
        swapSceneData(sceneData);

        const auto& reg = scene.registry();

        copyComponent<Camera>(reg, sceneData.currentCameras);
        copyComponent<Transform3D>(reg, sceneData.currentTransforms);
        copyComponent<StaticMeshComponent>(reg, sceneData.currentStaticMeshes);
        copyComponent<PointLight>(reg, sceneData.currentPointLights);
        copyComponent<SpotLight>(reg, sceneData.currentSpotLights);
        copyComponent<DirectionalLight>(reg, sceneData.currentDirectionalLights);
    }

    void SceneRenderer::renderSceneData(Graphics::CommandBuffer& commandBuffer,
                                        SceneData& sceneData) noexcept
    {
        /* Erase entities with components that are not present in the current scene */
        for (auto entity : entt::basic_view {sceneData.transformRenderInfo})
        {
            if (!sceneData.currentTransforms.contains(entity))
            {
                sceneData.transformRenderInfo.erase(entity);
            }
        }

        for (auto entity : entt::basic_view {sceneData.pointLightRenderInfo})
        {
            if (!sceneData.currentPointLights.contains(entity))
            {
                sceneData.pointLightRenderInfo.erase(entity);
            }
        }

        for (auto entity : entt::basic_view {sceneData.spotLightRenderInfo})
        {
            if (!sceneData.currentSpotLights.contains(entity))
            {
                sceneData.spotLightRenderInfo.erase(entity);
            }
        }

        sceneData.transformRenderInfo.reserve(sceneData.currentTransforms.size());

        entt::basic_view transformView {sceneData.currentTransforms};

        // Upload transform data
        for (auto entity : transformView)
        {
            auto& transform = transformView.get<Transform3D>(entity);
            if (!sceneData.transformRenderInfo.contains(entity))
            {
                sceneData.transformRenderInfo.emplace(entity);
            }

            auto& transformRenderInfo = sceneData.transformRenderInfo.get(entity);
            transformRenderInfo.data.model = transform.globalMatrix;
            transformRenderInfo.data.normal =
                glm::transpose(glm::inverse(transformRenderInfo.data.model));

            if (!transformRenderInfo.buffer)
            {
                Graphics::DynamicBufferCreateInfo bufferCreateInfo {_context};
                bufferCreateInfo.size = sizeof(TransformRenderInfo::Data);
                bufferCreateInfo.cached = false;

                transformRenderInfo.buffer = Graphics::DynamicFixedBuffer {bufferCreateInfo};
            }

            transformRenderInfo.buffer->write(
                std::as_bytes(std::span(&transformRenderInfo.data, 1)), 0);
            transformRenderInfo.buffer->update(commandBuffer,
                                               Graphics::PipelineStageFlags::eVertexShader,
                                               Graphics::AccessFlags::eShaderRead);
        }
    }

    void SceneRenderer::swapSceneData(SceneData& sceneData) noexcept
    {
        std::swap(sceneData.currentCameras, sceneData.previousCameras);
        std::swap(sceneData.currentTransforms, sceneData.previousTransforms);
        std::swap(sceneData.currentStaticMeshes, sceneData.previousStaticMeshes);
        std::swap(sceneData.currentPointLights, sceneData.previousPointLights);
        std::swap(sceneData.currentSpotLights, sceneData.previousSpotLights);
        std::swap(sceneData.currentDirectionalLights, sceneData.previousDirectionalLights);
    }
}  // namespace exage::Renderer