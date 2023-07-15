#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Sampler.h"
#include "exage/Graphics/Shader.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct DirectLightingSystemCreateInfo
    {
        Graphics::Context& context;
    };

    struct DirectLightingSystemRenderInfo
    {
        DirectionalLightRenderArray& directionalLightRenderArray;
        PointLightRenderArray& pointLightRenderArray;
        SpotLightRenderArray& spotLightRenderArray;
        std::shared_ptr<Graphics::Texture> position;
        std::shared_ptr<Graphics::Texture> normal;
        std::shared_ptr<Graphics::Texture> albedo;
        std::shared_ptr<Graphics::Texture> metallic;
        std::shared_ptr<Graphics::Texture> roughness;
        std::shared_ptr<Graphics::Texture> occlusion;
        std::shared_ptr<Graphics::Texture> emissive;
        glm::uvec2 extent;
    };

    class DirectLightingSystem
    {
      public:
        explicit DirectLightingSystem(const DirectLightingSystemCreateInfo& createInfo) noexcept;
        ~DirectLightingSystem() = default;

        EXAGE_DELETE_COPY(DirectLightingSystem);
        EXAGE_DEFAULT_MOVE(DirectLightingSystem);

        void render(Graphics::CommandBuffer& commandBuffer,
                    Scene& scene,
                    const DirectLightingSystemRenderInfo& renderInfo) noexcept;

      private:
        std::reference_wrapper<Graphics::Context> _context;

        std::shared_ptr<Graphics::Pipeline> _pipeline;
        std::shared_ptr<Graphics::Sampler> _sampler;

        std::shared_ptr<Graphics::Buffer> _vertexBuffer;
    };
}  // namespace exage::Renderer
