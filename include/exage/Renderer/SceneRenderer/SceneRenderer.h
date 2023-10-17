#pragma once

#include <glm/glm.hpp>

#include "exage/Graphics/Context.h"
#include "exage/Renderer/RenderSettings.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneData.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{
    struct SceneRendererCreateInfo
    {
        Graphics::Context& context;
        AssetCache& assetCache;
        RenderQualitySettings renderQualitySettings;
    };

    class SceneRenderer
    {
      public:
        explicit SceneRenderer(const SceneRendererCreateInfo& createInfo) noexcept;
        ~SceneRenderer() = default;

        EXAGE_DELETE_COPY_CONSTRUCT(SceneRenderer);
        EXAGE_DEFAULT_MOVE_CONSTRUCT(SceneRenderer);
        EXAGE_DELETE_ASSIGN(SceneRenderer);

        /* Copies relevant data from the scene to the scene data */
        void prepareSceneData(const Scene& scene, SceneData& sceneData) noexcept;
        void renderSceneData(Graphics::CommandBuffer& commandBuffer, SceneData& sceneData) noexcept;

      private:
        static void swapSceneData(SceneData& sceneData) noexcept;

        Graphics::Context& _context;
        AssetCache& _assetCache;
        RenderQualitySettings _renderQualitySettings;
    };
}  // namespace exage::Renderer