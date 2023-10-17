#pragma once

#include <glm/glm.hpp>

#include "exage/Graphics/Context.h"
#include "exage/Renderer/RenderSettings.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneData.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{
    struct CameraRendererCreateInfo
    {
        Graphics::Context& context;
        AssetCache& assetCache;
        glm::uvec2 extent;
        RenderQualitySettings renderQualitySettings;
    };

    class CameraRenderer
    {
      public:
        explicit CameraRenderer(const CameraRendererCreateInfo& createInfo) noexcept;
        ~CameraRenderer() = default;

        EXAGE_DELETE_COPY_CONSTRUCT(CameraRenderer);
        EXAGE_DEFAULT_MOVE_CONSTRUCT(CameraRenderer);
        EXAGE_DELETE_ASSIGN(CameraRenderer);

        void render(Graphics::CommandBuffer& commandBuffer,
                    SceneData& sceneData,
                    CameraData& cameraData,
                    Entity cameraEntity) noexcept;

        void resize(glm::uvec2 extent) noexcept;

      private:
        Graphics::Context& _context;
        AssetCache& _assetCache;
        glm::uvec2 _extent;
        RenderQualitySettings _renderQualitySettings;
    };
}  // namespace exage::Renderer