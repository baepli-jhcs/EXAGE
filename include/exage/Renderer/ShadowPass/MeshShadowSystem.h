#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct MeshShadowSystemCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
        AssetCache& assetCache;
    };

    class MeshShadowSystem
    {
      public:
        explicit MeshShadowSystem(const MeshShadowSystemCreateInfo& createInfo) noexcept;
        ~MeshShadowSystem() = default;

        EXAGE_DELETE_COPY(MeshShadowSystem);
        EXAGE_DEFAULT_MOVE(MeshShadowSystem);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene);

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;
        std::reference_wrapper<AssetCache> _assetCache;

        std::shared_ptr<Graphics::Pipeline> _pipeline;
    };
}  // namespace exage::Renderer