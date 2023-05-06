#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    [[nodiscard]] auto aabbInFrustum(const AABB& aabb,
                                     const glm::mat4& modelViewProjection) noexcept -> bool;

    class EXAGE_EXPORT MeshSystem
    {
      public:
        MeshSystem() = default;
        ~MeshSystem() = default;

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene);

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::reference_wrapper<SceneBuffer> _sceneBuffer;

        std::shared_ptr<Graphics::Pipeline> _pipeline;
    };
}  // namespace exage::Renderer
