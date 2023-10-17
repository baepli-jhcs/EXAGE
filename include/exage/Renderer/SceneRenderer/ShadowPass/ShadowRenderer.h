#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Renderer/RenderSettings.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/Scene/SceneData.h"
#include "exage/Renderer/SceneRenderer/ShadowPass/PointShadowSystem.h"

namespace exage::Renderer
{
    struct ShadowRendererCreateInfo
    {
        Graphics::Context& context;
        AssetCache& assetCache;
        RenderQualitySettings renderQualitySettings;
    };

    class ShadowRenderer
    {
      public:
        explicit ShadowRenderer(const ShadowRendererCreateInfo& createInfo) noexcept;
        ~ShadowRenderer() = default;

        EXAGE_DELETE_COPY_CONSTRUCT(ShadowRenderer);
        EXAGE_DEFAULT_MOVE_CONSTRUCT(ShadowRenderer);
        EXAGE_DELETE_ASSIGN(ShadowRenderer);

        void render(Graphics::CommandBuffer& commandBuffer, SceneData& scene) noexcept;
        void resize(glm::uvec2 extent) noexcept;

      private:
        void prepareLightingData(Graphics::CommandBuffer& commandBuffer,
                                 SceneData& sceneData) noexcept;

        Graphics::Context& _context;
        AssetCache& _assetCache;
        RenderQualitySettings _renderQualitySettings;

        PointShadowSystem _pointShadowSystem;
    };
}  // namespace exage::Renderer
