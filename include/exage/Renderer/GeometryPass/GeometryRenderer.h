#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Renderer/GeometryPass/MeshSystem.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct GeometryRendererCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
        glm::uvec2 extent;
        Graphics::Sampler::Anisotropy anisotropy = Graphics::Sampler::Anisotropy::e1;
    };

    class GeometryRenderer
    {
      public:
        explicit GeometryRenderer(const GeometryRendererCreateInfo& createInfo) noexcept;
        ~GeometryRenderer() = default;

        EXAGE_DELETE_COPY(GeometryRenderer);
        EXAGE_DEFAULT_MOVE(GeometryRenderer);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept;
        void resize(glm::uvec2 extent) noexcept;

        [[nodiscard]] auto getExtent() const noexcept -> const glm::uvec2& { return _extent; }
        [[nodiscard]] auto getFrameBuffer() const noexcept -> const Graphics::FrameBuffer&
        {
            return *_frameBuffer;
        }

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;
        glm::uvec2 _extent;

        MeshSystem _meshSystem;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;
    };
}  // namespace exage::Renderer
