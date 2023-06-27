#include <span>
#include <utility>

#include "exage/Renderer/ShadowPass/ShadowRenderer.h"

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Locations.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Utils/Frustum.h"
#include "exage/Scene/Hierarchy.h"

namespace exage::Renderer
{

    ShadowRenderer::ShadowRenderer(const ShadowRendererCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _sceneBuffer(createInfo.sceneBuffer)
        , _assetCache(createInfo.assetCache)
        , _shadowResolution(createInfo.shadowResolution)
        , _cascadeLevels(createInfo.cascadeLevels)
        , _directionalShadowSystem({_context, _sceneBuffer, _assetCache})
        , _pointShadowSystem({_context, _sceneBuffer, _assetCache})
    {
        _cascadeSplits.resize(static_cast<uint8_t>(_cascadeLevels));
    }

    void ShadowRenderer::render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept
    {
        _directionalShadowSystem.render(commandBuffer, scene);
        _pointShadowSystem.render(commandBuffer, scene);
    }

    void ShadowRenderer::prepareLightingData(Scene& scene,
                                             Graphics::CommandBuffer& commandBuffer) noexcept
    {
        auto& reg = scene.registry();
        auto& cameraRenderInfo = getCameraRenderInfo(scene);
        auto& camera = getCameraComponent(scene);

        {
            // Calculate split depths based on view camera frustum
            float clipRange = camera.far - camera.near;
            float minZ = camera.near;
            float maxZ = camera.near + clipRange;
            float range = maxZ - minZ;
            float ratio = maxZ / minZ;

            auto cascadeLevels = static_cast<uint8_t>(_cascadeLevels);

            for (uint8_t i = 0; i < cascadeLevels; ++i)
            {
                float p = (i + 1) / static_cast<float>(cascadeLevels);
                float log = minZ * std::pow(ratio, p);
                float uniform = minZ + range * p;
                float d = camera.near * std::exp(log * (1.0F - camera.near / camera.far));
                float f = camera.near + range * p;
                _cascadeSplits[i] = (d - camera.near) / clipRange;
            }

            auto view = entt::basic_view {reg.storage<DirectionalLight>(CURRENT_DIRECTIONAL_LIGHT)}
                | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

            auto& storage =
                reg.storage<DirectionalLightRenderInfo>(CURRENT_DIRECTIONAL_LIGHT_RENDER_INFO);

            auto& directionalLightArrayRenderInfo =
                reg.storage<DirectionalLightRenderArray>(CURRENT_DIRECTIONAL_LIGHT_RENDER_ARRAY);
            if (!directionalLightArrayRenderInfo.contains(scene.dataEntity()))
            {
                directionalLightArrayRenderInfo.emplace(scene.dataEntity());
            }
            auto& directionalLightArray = directionalLightArrayRenderInfo.get(scene.dataEntity());

            uint32_t size = 0;
            for (auto entity : view)
            {
                size++;
            }

            const auto bufferSize =
                16 + size * sizeof(DirectionalLightRenderArray::Data::ArrayItem);

            if (!directionalLightArray.buffer)
            {
                Graphics::DynamicBufferCreateInfo bufferCreateInfo {.context = _context.get()};
                bufferCreateInfo.cached = true;
                bufferCreateInfo.size = bufferSize;

                directionalLightArray.buffer = Graphics::ResizableDynamicBuffer(bufferCreateInfo);
            }

            directionalLightArray.buffer->resize(bufferSize);

            std::span<uint32_t> sizeSpan {&size, 1};
            directionalLightArray.buffer->write(std::as_bytes(sizeSpan), 0);

            uint32_t index = 0;
            uint32_t offset = 16;  // 4 bytes for size, 12 bytes for padding

            for (auto entity : view)
            {
                auto& directionalLight = view.get<DirectionalLight>(entity);
                auto& transform = view.get<Transform3D>(entity);

                if (!storage.contains(entity))
                {
                    storage.emplace(entity);
                }
                auto& directionalLightRenderInfo = storage.get(entity);

                if (directionalLight.castShadow)
                {
                    if (!directionalLightRenderInfo.shadowMap)
                    {
                        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                        frameBufferCreateInfo.extent = {_shadowResolution, _shadowResolution};
                        frameBufferCreateInfo.colorAttachments = {};

                        Graphics::TextureCreateInfo textureCreateInfo {};
                        textureCreateInfo.extent = {_shadowResolution, _shadowResolution, 1};
                        textureCreateInfo.format = _context.get().getHardwareSupport().depthFormat;
                        textureCreateInfo.usage =
                            Graphics::Texture::UsageFlags::eDepthStencilAttachment
                            | Graphics::Texture::UsageFlags::eSampled;
                        textureCreateInfo.type = Graphics::Texture::Type::e2D;
                        textureCreateInfo.arrayLayers = static_cast<uint32_t>(_cascadeLevels);
                        textureCreateInfo.mipLevels = 1;

                        auto texture = _context.get().createTexture(textureCreateInfo);

                        directionalLightRenderInfo.shadowMap =
                            _context.get().createFrameBuffer(frameBufferCreateInfo);

                        directionalLightRenderInfo.shadowMap->attachOrReplaceDepthStencil(texture);

                        directionalLightRenderInfo.shadowMapIndex = static_cast<int32_t>(
                            directionalLightRenderInfo.shadowMap->getDepthStencilTexture()
                                ->getBindlessID(Graphics::Texture::Aspect::eDepth)
                                .id);
                    }
                }
                else
                {
                    directionalLightRenderInfo.shadowMap.reset();
                    directionalLightRenderInfo.shadowMapIndex = -1;
                }

                // Calculate orthographic projection matrix for each cascade
                float lastSplitDist = 0.0F;
                auto cascadeLevels = static_cast<uint8_t>(_cascadeLevels);
                for (uint32_t i = 0; i < cascadeLevels; i++)
                {
                    float splitDist = _cascadeSplits[i];

                    auto frustumCorners =
                        getFrustumCorners(_context.get().getAPIProperties().depthZeroToOne);

                    // Project frustum corners into world space
                    glm::mat4 invCam = glm::inverse(cameraRenderInfo.data.viewProjection);
                    for (auto& frustumCorner : frustumCorners)
                    {
                        glm::vec4 invCorner = invCam * glm::vec4(frustumCorner, 1.0F);
                        frustumCorner = invCorner / invCorner.w;
                    }

                    for (uint32_t i = 0; i < 8; i++)
                    {
                        glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
                        frustumCorners[i] = invCorner / invCorner.w;
                    }

                    for (uint32_t i = 0; i < 4; i++)
                    {
                        glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
                        frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                        frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
                    }

                    // Get frustum center
                    glm::vec3 frustumCenter = glm::vec3(0.0F);
                    for (auto& frustumCorner : frustumCorners)
                    {
                        frustumCenter += frustumCorner;
                    }

                    frustumCenter /= 8.0F;

                    float radius = 0.0F;
                    for (auto& frustumCorner : frustumCorners)
                    {
                        float distance = glm::length(frustumCorner - frustumCenter);
                        radius = std::max(radius, distance);
                    }
                    radius = std::ceil(radius * 16.0F) / 16.0F;

                    glm::vec3 maxExtents = glm::vec3(radius);
                    glm::vec3 minExtents = -maxExtents;

                    glm::vec3 lightDir = glm::normalize(directionalLightRenderInfo.direction);
                    glm::mat4 lightView = glm::lookAt(frustumCenter - lightDir * -minExtents.z,
                                                      frustumCenter,
                                                      glm::vec3(0.0F, 1.0F, 0.0F));
                    glm::mat4 lightOrtho = glm::ortho(minExtents.x,
                                                      maxExtents.x,
                                                      minExtents.y,
                                                      maxExtents.y,
                                                      0.0F,
                                                      maxExtents.z - minExtents.z);
                    glm::mat4 lightProjection = lightOrtho * lightView;

                    directionalLightRenderInfo.cascadeViewProjections[i] = lightProjection;
                    directionalLightRenderInfo.cascadeSplits[i] =
                        (camera.near + splitDist * clipRange) * -1.0F;
                    lastSplitDist = _cascadeSplits[i];
                }

                directionalLightRenderInfo.arrayIndex = index;

                DirectionalLightRenderArray::Data::ArrayItem arrayItem {};
                arrayItem.direction = transform.globalRotation.getForwardVector();
                arrayItem.color = directionalLight.color;
                arrayItem.intensity = directionalLight.intensity;
                arrayItem.shadowMapIndex = directionalLightRenderInfo.shadowMapIndex;
                arrayItem.shadowBias = directionalLight.shadowBias;
                arrayItem.cascadeLevels = static_cast<uint8_t>(_cascadeLevels);
                for (uint8_t i = 0; i < static_cast<uint8_t>(_cascadeLevels); i++)
                {
                    arrayItem.cascadeViewProjections[i] =
                        directionalLightRenderInfo.cascadeViewProjections[i];
                    arrayItem.cascadeSplits[i] = directionalLightRenderInfo.cascadeSplits[i];
                }

                std::span<DirectionalLightRenderArray::Data::ArrayItem> arrayItemSpan {&arrayItem,
                                                                                       1};
                directionalLightArray.buffer->write(std::as_bytes(arrayItemSpan), offset);

                directionalLightRenderInfo.direction = arrayItem.direction;
                directionalLightRenderInfo.color = arrayItem.color;
                directionalLightRenderInfo.intensity = arrayItem.intensity;
                directionalLightRenderInfo.shadowBias = arrayItem.shadowBias;

                index++;
                offset += sizeof(DirectionalLightRenderArray::Data::ArrayItem);
            }

            directionalLightArray.buffer->update(commandBuffer,
                                                 Graphics::PipelineStageFlags::eFragmentShader,
                                                 Graphics::AccessFlags::eShaderRead);

            auto renderInfoView = entt::runtime_view {};
            renderInfoView.iterate(
                reg.storage<DirectionalLightRenderInfo>(CURRENT_DIRECTIONAL_LIGHT_RENDER_INFO));
            renderInfoView.exclude(reg.storage<DirectionalLight>(CURRENT_DIRECTIONAL_LIGHT));

            // Remove unused directional light render info
            for (auto entity : renderInfoView)
            {
                storage.erase(entity);
            }
        }

        {
            auto view = entt::basic_view {reg.storage<PointLight>(CURRENT_POINT_LIGHT)}
                | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

            auto& storage = reg.storage<PointLightRenderInfo>(CURRENT_POINT_LIGHT_RENDER_INFO);
            auto& pointLightArrayRenderInfo =
                reg.storage<PointLightRenderArray>(CURRENT_POINT_LIGHT_RENDER_ARRAY);
            if (!pointLightArrayRenderInfo.contains(scene.dataEntity()))
            {
                pointLightArrayRenderInfo.emplace(scene.dataEntity());
            }
            auto& pointLightArray = pointLightArrayRenderInfo.get(scene.dataEntity());

            uint32_t size = 0;
            for (auto entity : view)
            {
                size++;
            }

            uint32_t bufferSize = 16 + size * sizeof(PointLightRenderArray::Data::ArrayItem);

            if (!pointLightArray.buffer)
            {
                Graphics::DynamicBufferCreateInfo bufferCreateInfo {.context = _context.get()};
                bufferCreateInfo.cached = true;
                bufferCreateInfo.size = bufferSize;

                pointLightArray.buffer = Graphics::ResizableDynamicBuffer(bufferCreateInfo);
            }

            pointLightArray.buffer->resize(bufferSize);

            std::span<uint32_t> sizeSpan {&size, 1};
            pointLightArray.buffer->write(std::as_bytes(sizeSpan), 0);

            uint32_t index = 0;
            uint32_t offset = 16;

            for (auto entity : view)
            {
                auto& pointLight = view.get<PointLight>(entity);
                auto& transform = view.get<Transform3D>(entity);

                if (!storage.contains(entity))
                {
                    storage.emplace(entity);
                }
                auto& pointLightRenderInfo = storage.get(entity);

                if (!pointLightRenderInfo.shadowMap)
                {
                    Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                    frameBufferCreateInfo.extent = {_shadowResolution, _shadowResolution};
                    frameBufferCreateInfo.colorAttachments = {};

                    Graphics::TextureCreateInfo textureCreateInfo {};
                    textureCreateInfo.extent = {_shadowResolution, _shadowResolution, 1};
                    textureCreateInfo.format = _context.get().getHardwareSupport().depthFormat;
                    textureCreateInfo.usage = Graphics::Texture::UsageFlags::eDepthStencilAttachment
                        | Graphics::Texture::UsageFlags::eSampled;
                    textureCreateInfo.type = Graphics::Texture::Type::eCube;
                    textureCreateInfo.arrayLayers = 6;
                    textureCreateInfo.mipLevels = 1;

                    auto texture = _context.get().createTexture(textureCreateInfo);

                    pointLightRenderInfo.shadowMap =
                        _context.get().createFrameBuffer(frameBufferCreateInfo);

                    pointLightRenderInfo.shadowMap->attachOrReplaceDepthStencil(texture);

                    pointLightRenderInfo.shadowMapIndex = static_cast<int32_t>(
                        pointLightRenderInfo.shadowMap->getDepthStencilTexture()
                            ->getBindlessID(Graphics::Texture::Aspect::eDepth)
                            .id);
                }

                pointLightRenderInfo.arrayIndex = index;

                PointLightRenderArray::Data::ArrayItem arrayItem {};
                arrayItem.position = transform.position;
                arrayItem.color = pointLight.color;
                arrayItem.intensity = pointLight.intensity;
                arrayItem.physicalRadius = pointLight.physicalRadius;
                arrayItem.attenuationRadius = pointLight.attenuationRadius;
                arrayItem.shadowMapIndex = pointLightRenderInfo.shadowMapIndex;
                arrayItem.shadowBias = pointLight.shadowBias;

                std::span<PointLightRenderArray::Data::ArrayItem> arrayItemSpan {&arrayItem, 1};
                pointLightArray.buffer->write(std::as_bytes(arrayItemSpan), offset);

                pointLightRenderInfo.position = arrayItem.position;
                pointLightRenderInfo.color = arrayItem.color;
                pointLightRenderInfo.intensity = arrayItem.intensity;
                pointLightRenderInfo.physicalRadius = arrayItem.physicalRadius;
                pointLightRenderInfo.attenuationRadius = arrayItem.attenuationRadius;
                pointLightRenderInfo.shadowBias = arrayItem.shadowBias;

                index++;
                offset += sizeof(PointLightRenderArray::Data::ArrayItem);
            }

            pointLightArray.buffer->update(commandBuffer,
                                           Graphics::PipelineStageFlags::eFragmentShader,
                                           Graphics::AccessFlags::eShaderRead);

            auto renderInfoView = entt::runtime_view {};
            renderInfoView.iterate(
                reg.storage<PointLightRenderInfo>(CURRENT_POINT_LIGHT_RENDER_INFO));
            renderInfoView.exclude(reg.storage<PointLight>(CURRENT_POINT_LIGHT));

            // Remove unused point light render info
            for (auto entity : renderInfoView)
            {
                storage.erase(entity);
            }
        }

        {
            auto view = entt::basic_view {reg.storage<SpotLight>(CURRENT_SPOT_LIGHT)}
                | entt::basic_view {reg.storage<Transform3D>(CURRENT_TRANSFORM_3D)};

            auto& storage = reg.storage<SpotLightRenderInfo>(CURRENT_SPOT_LIGHT_RENDER_INFO);
            auto& spotLightArrayRenderInfo =
                reg.storage<SpotLightRenderArray>(CURRENT_SPOT_LIGHT_RENDER_ARRAY);
            if (!spotLightArrayRenderInfo.contains(scene.dataEntity()))
            {
                spotLightArrayRenderInfo.emplace(scene.dataEntity());
            }
            auto& spotLightArray = spotLightArrayRenderInfo.get(scene.dataEntity());

            uint32_t size = 0;
            for (auto entity : view)
            {
                size++;
            }

            const auto bufferSize = 16 + size * sizeof(SpotLightRenderArray::Data::ArrayItem);

            if (!spotLightArray.buffer)
            {
                Graphics::DynamicBufferCreateInfo bufferCreateInfo {.context = _context.get()};
                bufferCreateInfo.cached = true;
                bufferCreateInfo.size = bufferSize;

                spotLightArray.buffer = Graphics::ResizableDynamicBuffer(bufferCreateInfo);
            }

            spotLightArray.buffer->resize(bufferSize);

            std::span<uint32_t> sizeSpan {&size, 1};
            spotLightArray.buffer->write(std::as_bytes(sizeSpan), 0);

            uint32_t index = 0;
            uint32_t offset = 16;

            for (auto entity : view)
            {
                auto& spotLight = view.get<SpotLight>(entity);
                auto& transform = view.get<Transform3D>(entity);

                if (!storage.contains(entity))
                {
                    storage.emplace(entity);
                }
                auto& spotLightRenderInfo = storage.get(entity);

                if (!spotLightRenderInfo.shadowMap)
                {
                    Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
                    frameBufferCreateInfo.extent = {_shadowResolution, _shadowResolution};
                    frameBufferCreateInfo.colorAttachments = {};
                    frameBufferCreateInfo.depthAttachment = {
                        _context.get().getHardwareSupport().depthFormat,
                        Graphics::Texture::UsageFlags::eDepthStencilAttachment
                            | Graphics::Texture::UsageFlags::eSampled};

                    spotLightRenderInfo.shadowMap =
                        _context.get().createFrameBuffer(frameBufferCreateInfo);
                    spotLightRenderInfo.shadowMapIndex =
                        spotLightRenderInfo.shadowMap->getDepthStencilTexture()
                            ->getBindlessID(Graphics::Texture::Aspect::eDepth)
                            .id;
                }

                spotLightRenderInfo.arrayIndex = index;

                SpotLightRenderArray::Data::ArrayItem arrayItem {};
                arrayItem.position = transform.position;
                arrayItem.direction = glm::normalize(transform.rotation.getForwardVector());
                arrayItem.color = spotLight.color;
                arrayItem.intensity = spotLight.intensity;
                arrayItem.innerCutoff = spotLight.innerCutoff;
                arrayItem.outerCutoff = spotLight.outerCutoff;
                arrayItem.physicalRadius = spotLight.physicalRadius;
                arrayItem.attenuationRadius = spotLight.attenuationRadius;
                arrayItem.shadowMapIndex = spotLightRenderInfo.shadowMapIndex;
                arrayItem.shadowBias = spotLight.shadowBias;

                std::span<SpotLightRenderArray::Data::ArrayItem> arrayItemSpan {&arrayItem, 1};
                spotLightArray.buffer->write(std::as_bytes(arrayItemSpan), offset);

                index++;
                offset += sizeof(SpotLightRenderArray::Data::ArrayItem);
            }

            auto renderInfoView = entt::runtime_view {};
            renderInfoView.iterate(
                reg.storage<SpotLightRenderInfo>(CURRENT_SPOT_LIGHT_RENDER_INFO));
            renderInfoView.exclude(reg.storage<SpotLight>(CURRENT_SPOT_LIGHT));

            // Remove unused spot light render info
            for (auto entity : renderInfoView)
            {
                storage.erase(entity);
            }
        }
    }

}  // namespace exage::Renderer