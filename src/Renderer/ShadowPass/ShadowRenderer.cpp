#include <span>
#include <utility>

#include "exage/Renderer/SceneRenderer/ShadowPass/ShadowRenderer.h"

#include <entt/entity/fwd.hpp>

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Utils/Frustum.h"
#include "exage/Scene/Hierarchy.h"

namespace exage::Renderer
{

    ShadowRenderer::ShadowRenderer(const ShadowRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _assetCache(createInfo.assetCache)
        , _renderQualitySettings(createInfo.renderQualitySettings)
        , _pointShadowSystem({_context, _assetCache})
    {
    }

    void ShadowRenderer::render(Graphics::CommandBuffer& commandBuffer,
                                SceneData& sceneData) noexcept
    {
        prepareLightingData(commandBuffer, sceneData);

        _pointShadowSystem.render(commandBuffer, sceneData);
    }

    void ShadowRenderer::prepareLightingData(Graphics::CommandBuffer& commandBuffer,
                                             SceneData& sceneData) noexcept
    {
        {
            entt::runtime_view view {};
            view.iterate(sceneData.pointLightRenderInfo);
            view.exclude(sceneData.currentPointLights);

            for (auto entity : view)
            {
                sceneData.pointLightRenderInfo.erase(entity);
            }
        }

        {
            entt::runtime_view view {};
            view.iterate(sceneData.spotLightRenderInfo);
            view.exclude(sceneData.currentSpotLights);

            for (auto entity : view)
            {
                sceneData.spotLightRenderInfo.erase(entity);
            }
        }

        sceneData.pointLightRenderInfo.reserve(sceneData.currentPointLights.size());
        sceneData.spotLightRenderInfo.reserve(sceneData.currentSpotLights.size());
    }

}  // namespace exage::Renderer