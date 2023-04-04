#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Renderer
{
    struct RendererCreateInfo
    {
        Graphics::Context& context;
        glm::uvec2 extent;
    };

    class EXAGE_EXPORT Renderer
    {
      public:
        explicit Renderer(const RendererCreateInfo& createInfo) noexcept;
        virtual ~Renderer() = default;

        EXAGE_DELETE_COPY(Renderer);
        EXAGE_DEFAULT_MOVE(Renderer);

        void render(Graphics::CommandBuffer& commandBuffer) noexcept;

        void resize(const glm::uvec2& extent) noexcept;
        [[nodiscard]] auto getExtent() const noexcept -> const glm::uvec2& { return _extent; }

      private:
        std::reference_wrapper<Graphics::Context> _context;
        glm::uvec2 _extent;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;
    };
}  // namespace exage::Renderer
