#pragma once

#include <functional>

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/GeometryPass/GeometryRenderer.h"
#include "exage/Renderer/LightingPass/LightingRenderer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/ShadowPass/ShadowRenderer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct RendererCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
        glm::uvec2 extent;
        Graphics::Sampler::Anisotropy anisotropy = Graphics::Sampler::Anisotropy::e1;
        ShadowResolution shadowResolution = ShadowResolution::e1024;
        CascadeLevels cascadeLevels = CascadeLevels::e4;
    };

    class Renderer
    {
      public:
        explicit Renderer(const RendererCreateInfo& createInfo) noexcept;
        virtual ~Renderer() = default;

        EXAGE_DELETE_COPY(Renderer);
        EXAGE_DEFAULT_MOVE(Renderer);

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

        Graphics::Sampler::Anisotropy _anisotropy;
        ShadowResolution _shadowResolution;
        CascadeLevels _cascadeLevels;

        GeometryRenderer _geometryRenderer;
        ShadowRenderer _shadowRenderer;
        LightingRenderer _lightingRenderer;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;
    };

    void copySceneForRenderer(Scene& scene) noexcept;
}  // namespace exage::Renderer
