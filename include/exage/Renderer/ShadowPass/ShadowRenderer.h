#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/ShadowPass/DirectionalShadowSystem.h"
#include "exage/Renderer/ShadowPass/PointShadowSystem.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    enum class ShadowResolution : uint32_t
    {
        e256 = 256,
        e512 = 512,
        e1024 = 1024,
        e2048 = 2048,
        e4096 = 4096,
    };

    struct ShadowRendererCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
        ShadowResolution shadowResolution;
        CascadeLevels cascadeLevels;
    };

    class ShadowRenderer
    {
      public:
        explicit ShadowRenderer(const ShadowRendererCreateInfo& createInfo) noexcept;
        ~ShadowRenderer() = default;

        EXAGE_DELETE_COPY(ShadowRenderer);
        EXAGE_DEFAULT_MOVE(ShadowRenderer);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept;
        void resize(glm::uvec2 extent) noexcept;

        [[nodiscard]] auto getShadowResolution() const noexcept -> ShadowResolution
        {
            return _shadowResolution;
        }

        void prepareLightingData(Scene& scene, Graphics::CommandBuffer& commandBuffer) noexcept;

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;
        ShadowResolution _shadowResolution;
        CascadeLevels _cascadeLevels;

        DirectionalShadowSystem _directionalShadowSystem;
        PointShadowSystem _pointShadowSystem;

        std::vector<float> _cascadeSplits;
    };
}  // namespace exage::Renderer
