#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/Scene/SceneData.h"

namespace exage::Renderer
{
    struct PointShadowSystemCreateInfo
    {
        Graphics::Context& context;
        AssetCache& assetCache;
    };

    class PointShadowSystem
    {
      public:
        explicit PointShadowSystem(const PointShadowSystemCreateInfo& createInfo) noexcept;
        ~PointShadowSystem() = default;

        EXAGE_DELETE_COPY(PointShadowSystem);
        EXAGE_DEFAULT_MOVE(PointShadowSystem);

        void render(Graphics::CommandBuffer& commandBuffer, SceneData& sceneData) noexcept;

      private:
        void renderShadow(Graphics::CommandBuffer& commandBuffer,
                          SceneData& sceneData,
                          Transform3D& transform,
                          PointLight& light) noexcept;

        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;

        std::shared_ptr<Graphics::Pipeline> _pipeline;

        std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Renderer