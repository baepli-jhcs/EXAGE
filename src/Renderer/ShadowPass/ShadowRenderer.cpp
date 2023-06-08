#include "exage/Renderer/ShadowPass/ShadowRenderer.h"

#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Locations.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Scene/Hierarchy.h"

namespace exage::Renderer
{

    ShadowRenderer::ShadowRenderer(const ShadowRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _sceneBuffer(createInfo.sceneBuffer)
        , _assetCache(createInfo.assetCache)
        , _shadowResolution(createInfo.shadowResolution)
    {
    }

    void ShadowRenderer::prepareLightingData(Scene& scene) noexcept
    {
        auto& reg = scene.registry();
        {
            auto view = entt::basic_view {reg.storage<PointLight>(CURRENT_POINT_LIGHT)}
                | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

            auto& storage = reg.storage<PointLightRenderInfo>(CURRENT_POINT_LIGHT_RENDER_INFO);

            for (auto entity : view)
            {
                auto& pointLight = view.get<PointLight>(entity);
                auto& transform = view.get<Transform3D>(entity);

                if (!storage.contains(entity))
                {
                    storage.emplace(entity);
                }
                auto& pointLightRenderInfo = storage.get(entity);
                pointLightRenderInfo.data.position = transform.position;
                pointLightRenderInfo.data.color = pointLight.color;

                if (!pointLightRenderInfo.shadowMap
                    || pointLightRenderInfo.shadowMap->getExtent().x
                        != static_cast<uint32_t>(_shadowResolution))
                {
                    Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                    frameBufferCreateInfo.extent = {_shadowResolution, _shadowResolution};
                    frameBufferCreateInfo.colorAttachments = {};
                    frameBufferCreateInfo.depthAttachment = {
                        _context.get().getHardwareSupport().depthFormat,
                        Graphics::Texture::UsageFlags::eDepthStencilAttachment};

                    pointLightRenderInfo.shadowMap =
                        _context.get().createFrameBuffer(frameBufferCreateInfo);
                    pointLightRenderInfo.data.shadowMapIndex =
                        pointLightRenderInfo.shadowMap->getDepthStencilTexture()
                            ->getBindlessID()
                            .id;
                }
            }
        }

        {
            auto view = entt::basic_view {reg.storage<DirectionalLight>(CURRENT_DIRECTIONAL_LIGHT)}
                | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

            auto& storage =
                reg.storage<DirectionalLightRenderInfo>(CURRENT_DIRECTIONAL_LIGHT_RENDER_INFO);

            for (auto entity : view)
            {
                auto& directionalLight = view.get<DirectionalLight>(entity);
                auto& transform = view.get<Transform3D>(entity);

                if (!storage.contains(entity))
                {
                    storage.emplace(entity);
                }
                auto& directionalLightRenderInfo = storage.get(entity);
                directionalLightRenderInfo.data.direction = transform.rotation.getForwardVector();
                directionalLightRenderInfo.data.color = directionalLight.color;

                if (!directionalLightRenderInfo.shadowMap
                    || directionalLightRenderInfo.shadowMap->getExtent().x
                        != static_cast<uint32_t>(_shadowResolution))
                {
                    Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                    frameBufferCreateInfo.extent = {_shadowResolution, _shadowResolution};
                    frameBufferCreateInfo.colorAttachments = {};
                    frameBufferCreateInfo.depthAttachment = {
                        _context.get().getHardwareSupport().depthFormat,
                        Graphics::Texture::UsageFlags::eDepthStencilAttachment};

                    directionalLightRenderInfo.shadowMap =
                        _context.get().createFrameBuffer(frameBufferCreateInfo);
                    directionalLightRenderInfo.data.shadowMapIndex =
                        directionalLightRenderInfo.shadowMap->getDepthStencilTexture()
                            ->getBindlessID()
                            .id;
                }
            }
        }

        {
            auto view = entt::basic_view {reg.storage<SpotLight>(CURRENT_SPOT_LIGHT)}
                | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

            auto& storage = reg.storage<SpotLightRenderInfo>(CURRENT_SPOT_LIGHT_RENDER_INFO);

            for (auto entity : view)
            {
                auto& spotLight = view.get<SpotLight>(entity);
                auto& transform = view.get<Transform3D>(entity);

                if (!storage.contains(entity))
                {
                    storage.emplace(entity);
                }
                auto& spotLightRenderInfo = storage.get(entity);
                spotLightRenderInfo.data.position = transform.position;
                spotLightRenderInfo.data.direction = transform.rotation.getForwardVector();
                spotLightRenderInfo.data.color = spotLight.color;
                spotLightRenderInfo.data.innerCutoff = spotLight.innerCutoff;
                spotLightRenderInfo.data.outerCutoff = spotLight.outerCutoff;

                if (!spotLightRenderInfo.shadowMap
                    || spotLightRenderInfo.shadowMap->getExtent().x
                        != static_cast<uint32_t>(_shadowResolution))
                {
                    Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                    frameBufferCreateInfo.extent = {_shadowResolution, _shadowResolution};
                    frameBufferCreateInfo.colorAttachments = {};
                    frameBufferCreateInfo.depthAttachment = {
                        _context.get().getHardwareSupport().depthFormat,
                        Graphics::Texture::UsageFlags::eDepthStencilAttachment};

                    spotLightRenderInfo.shadowMap =
                        _context.get().createFrameBuffer(frameBufferCreateInfo);
                    spotLightRenderInfo.data.shadowMapIndex =
                        spotLightRenderInfo.shadowMap->getDepthStencilTexture()->getBindlessID().id;
                }
            }
        }
    }

}  // namespace exage::Renderer