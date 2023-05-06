#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct ForwardRendererCreateInfo
    {
        Graphics::Context& context;
        std::shared_ptr<Graphics::ResourceManager> resourceManager;
        glm::uvec2 extent;
    };

    class EXAGE_EXPORT ForwardRenderer
    {
      public:
        explicit ForwardRenderer(const ForwardRendererCreateInfo& createInfo) noexcept;
        virtual ~ForwardRenderer() = default;

        EXAGE_DELETE_COPY(ForwardRenderer);
        EXAGE_DEFAULT_MOVE(ForwardRenderer);

        void render(Graphics::CommandBuffer& commandBuffer, Scene& scene) noexcept;
        void resize(glm::uvec2 extent) noexcept;

        [[nodiscard]] auto getExtent() const noexcept -> const glm::uvec2& { return _extent; }
        [[nodiscard]] auto getFrameBuffer() const noexcept -> const Graphics::FrameBuffer&
        {
            return *_frameBuffer;
        }

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::shared_ptr<Graphics::ResourceManager> _resourceManager;
        glm::uvec2 _extent;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;
    };
}  // namespace exage::Renderer
