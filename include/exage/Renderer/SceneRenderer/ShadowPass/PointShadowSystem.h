#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/RenderSettings.h"
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
        RenderQualitySettings renderQualitySettings;
    };

    class PointShadowSystem
    {
      public:
        explicit PointShadowSystem(const PointShadowSystemCreateInfo& createInfo) noexcept;
        ~PointShadowSystem() = default;

        EXAGE_DELETE_COPY_CONSTRUCT(PointShadowSystem);
        EXAGE_DEFAULT_MOVE_CONSTRUCT(PointShadowSystem);
        EXAGE_DELETE_ASSIGN(PointShadowSystem);

        void render(Graphics::CommandBuffer& commandBuffer, SceneData& sceneData) noexcept;

      private:
        void renderShadow(Graphics::CommandBuffer& commandBuffer,
                          SceneData& sceneData,
                          Transform3D& transform,
                          PointLight& light,
                          PointLightRenderInfo& info) noexcept;

        [[nodiscard]] auto shouldLightRenderShadow(const Transform3D& transform,
                                                   const PointLight& light,
                                                   PointLightRenderInfo& info) noexcept -> bool;

        Graphics::Context& _context;
        AssetCache& _assetCache;
        RenderQualitySettings _renderQualitySettings;

        std::vector<PointLightRenderArray::Data::ArrayItem> _pointLightRenderArrayData;

        std::shared_ptr<Graphics::Pipeline> _pipeline;

        std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Renderer