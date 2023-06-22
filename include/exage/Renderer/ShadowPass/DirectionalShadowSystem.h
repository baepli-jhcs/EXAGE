#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    enum class CascadeLevels : uint8_t
    {
        e1 = 1,
        e2 = 2,
        e3 = 3,
        e4 = 4,
        e5 = 5,
    };

    struct DirectionalShadowSystemCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
        CascadeLevels cascadeLevels;
    };

    class DirectionalShadowSystem
    {
      public:
        explicit DirectionalShadowSystem(
            const DirectionalShadowSystemCreateInfo& createInfo) noexcept;
        ~DirectionalShadowSystem() = default;

        EXAGE_DELETE_COPY(DirectionalShadowSystem);
        EXAGE_DEFAULT_MOVE(DirectionalShadowSystem);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene);

      private:
        void renderShadow(Graphics::CommandBuffer& commandBuffer,
                          Scene& scene,
                          Camera& camera,
                          CameraRenderInfo& cameraRenderInfo,
                          DirectionalLightRenderInfo& lightRenderInfo) noexcept;

        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;
        CascadeLevels _cascadeLevels;

        std::shared_ptr<Graphics::Pipeline> _pipeline;

        std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Renderer