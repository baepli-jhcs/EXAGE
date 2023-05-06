#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/ResourceManager.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Graphics/Utils/RAII.h"
#include "exage/Renderer/GeometryPass/GeometryRenderer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct RendererCreateInfo
    {
        Graphics::Context& context;
        std::shared_ptr<Graphics::ResourceManager> resourceManager = nullptr;  // Can be nullptr
        glm::uvec2 extent;
    };

    class EXAGE_EXPORT Renderer
    {
      public:
        explicit Renderer(const RendererCreateInfo& createInfo) noexcept;
        virtual ~Renderer() = default;

        EXAGE_DELETE_COPY(Renderer);
        EXAGE_DEFAULT_MOVE(Renderer);

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

        ForwardRenderer _forwardRenderer;

        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;

        Graphics::DynamicFixedBuffer _cameraBuffer;
        std::shared_ptr<Graphics::RAII::BufferID> _cameraBufferID;
        Graphics::ResizableDynamicBuffer _transformBuffer;
        std::shared_ptr<Graphics::RAII::BufferID> _transformBufferID;
    };
}  // namespace exage::Renderer
