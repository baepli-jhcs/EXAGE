#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/LightingPass/DirectLightingSystem.h"

namespace exage::Renderer
{
    struct LightingRendererCreateInfo
    {
        Graphics::Context& context;
        glm::uvec2 extent;
    };

    struct LightingRenderInfo
    {
        std::shared_ptr<Graphics::Texture> position;
        std::shared_ptr<Graphics::Texture> normal;
        std::shared_ptr<Graphics::Texture> albedo;
        std::shared_ptr<Graphics::Texture> metallic;
        std::shared_ptr<Graphics::Texture> roughness;
        std::shared_ptr<Graphics::Texture> occlusion;
        std::shared_ptr<Graphics::Texture> emissive;
    };

    class LightingRenderer
    {
      public:
        explicit LightingRenderer(const LightingRendererCreateInfo& createInfo) noexcept;
        ~LightingRenderer() = default;

        EXAGE_DELETE_COPY(LightingRenderer);
        EXAGE_DEFAULT_MOVE(LightingRenderer);

        void render(Graphics::CommandBuffer& commandBuffer,
                    Scene& scene,
                    const LightingRenderInfo& renderInfo) noexcept;
        void resize(glm::uvec2 extent) noexcept;

        [[nodiscard]] auto getExtent() const noexcept -> const glm::uvec2& { return _extent; }
        [[nodiscard]] auto getFrameBuffer() const noexcept -> const Graphics::FrameBuffer&
        {
            return *_frameBuffer;
        }

      private:
        std::reference_wrapper<Graphics::Context> _context;
        glm::uvec2 _extent;

        DirectLightingSystem _directLightingSystem;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;
    };

}  // namespace exage::Renderer