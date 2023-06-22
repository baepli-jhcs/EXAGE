#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Sampler.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{

    struct MeshSystemCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
        Graphics::Sampler::Anisotropy anisotropy = Graphics::Sampler::Anisotropy::e1;
    };

    class MeshSystem
    {
      public:
        explicit MeshSystem(const MeshSystemCreateInfo& createInfo) noexcept;
        ~MeshSystem() = default;

        EXAGE_DELETE_COPY(MeshSystem);
        EXAGE_DEFAULT_MOVE(MeshSystem);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene, glm::uvec2 extent);

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;

        std::shared_ptr<Graphics::Pipeline> _pipeline;
        std::shared_ptr<Graphics::Sampler> _sampler;

        std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Renderer
