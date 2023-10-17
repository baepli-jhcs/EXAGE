#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Renderer/CameraRenderer/GeometryPass/MeshSystem.h"
#include "exage/Renderer/RenderSettings.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/Scene/SceneData.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct GeometryRendererCreateInfo
    {
        Graphics::Context& context;
        AssetCache& assetCache;
        glm::uvec2 extent;
        RenderQualitySettings renderQualitySettings;
    };

    class GeometryRenderer
    {
      public:
        explicit GeometryRenderer(const GeometryRendererCreateInfo& createInfo) noexcept;
        ~GeometryRenderer() = default;

        EXAGE_DELETE_COPY_CONSTRUCT(GeometryRenderer);
        EXAGE_DEFAULT_MOVE_CONSTRUCT(GeometryRenderer);
        EXAGE_DELETE_ASSIGN(GeometryRenderer);

        void render(Graphics::CommandBuffer& commandBuffer,
                    SceneData& sceneData,
                    CameraData& cameraData) noexcept;
        void resize(glm::uvec2 extent) noexcept;

        [[nodiscard]] auto getExtent() const noexcept -> const glm::uvec2& { return _extent; }
        [[nodiscard]] auto getFrameBuffer() const noexcept -> const Graphics::FrameBuffer&
        {
            return *_frameBuffer;
        }

      private:
        Graphics::Context& _context;
        AssetCache& _assetCache;
        glm::uvec2 _extent;
        RenderQualitySettings _renderQualitySettings;

        MeshSystem _meshSystem;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;
    };
}  // namespace exage::Renderer
