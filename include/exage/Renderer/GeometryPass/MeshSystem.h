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

    struct MeshSystemCreateInfo
    {
        Graphics::Context& context;
        SceneBuffer& sceneBuffer;
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

        std::shared_ptr<Graphics::Pipeline> _pipeline;
    };
}  // namespace exage::Renderer
