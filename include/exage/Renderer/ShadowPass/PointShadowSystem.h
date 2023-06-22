#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct PointShadowSystemCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
    };

    class PointShadowSystem
    {
      public:
        explicit PointShadowSystem(const PointShadowSystemCreateInfo& createInfo) noexcept;
        ~PointShadowSystem() = default;

        EXAGE_DELETE_COPY(PointShadowSystem);
        EXAGE_DEFAULT_MOVE(PointShadowSystem);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene);

      private:
        void renderShadow(Graphics::CommandBuffer& commandBuffer,
                          Scene& scene,
                          PointLightRenderInfo& pointLightRenderInfo,
                          Transform3D& transform,
                          PointLight& pointLight);

        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;

        std::shared_ptr<Graphics::Pipeline> _pipeline;

        std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Renderer