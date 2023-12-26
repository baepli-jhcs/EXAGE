#pragma once

#include <exage/GUI/ImGui.h>
#include <exage/Graphics/CommandBuffer.h>
#include <exage/Projects/Level.h>

namespace exitor
{
    using namespace exage;

    class LevelPanel
    {
      public:
        LevelPanel(Graphics::Context& context, Projects::Level& level) noexcept;
        ~LevelPanel() = default;

        void setLevel(Projects::Level& level) noexcept;

        EXAGE_DELETE_COPY(LevelPanel);
        EXAGE_DEFAULT_MOVE(LevelPanel);

        void run(Graphics::CommandBuffer& commandBuffer, float deltaTime) noexcept;

      private:
        Graphics::Context* _context;
        Projects::Level* _level;
        glm::uvec2 _viewportExtent;

        std::shared_ptr<Graphics::Texture> _testTexture;  // TODO: remove
        std::shared_ptr<Graphics::Sampler> _sampler;
        GUI::ImGui::Texture _imTexture;
    };
}  // namespace exitor