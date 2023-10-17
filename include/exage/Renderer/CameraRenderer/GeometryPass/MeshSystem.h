#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Sampler.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/RenderSettings.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{

    struct MeshSystemCreateInfo
    {
        Graphics::Context& context;
        AssetCache& assetCache;
        RenderQualitySettings renderQualitySettings;
    };

    class MeshSystem
    {
      public:
        explicit MeshSystem(const MeshSystemCreateInfo& createInfo) noexcept;
        ~MeshSystem() = default;

        EXAGE_DELETE_COPY_CONSTRUCT(MeshSystem);
        EXAGE_DEFAULT_MOVE_CONSTRUCT(MeshSystem);
        EXAGE_DELETE_ASSIGN(MeshSystem);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene, glm::uvec2 extent);

      private:
        Graphics::Context& _context;
        AssetCache& _assetCache;

        std::shared_ptr<Graphics::Pipeline> _pipeline;
        std::shared_ptr<Graphics::Sampler> _sampler;

        std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Renderer
