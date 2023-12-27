#pragma once

#include <exage/GUI/Fonts.h>
#include <exage/GUI/ImGui.h>
#include <exage/Graphics/CommandBuffer.h>
#include <exage/Projects/Level.h>

#include "imgui.h"

namespace exitor
{
    using namespace exage;

    class LevelPanel
    {
      public:
        LevelPanel(Graphics::Context& context,
                   GUI::ImGui::FontManager& fontManager,
                   Projects::Level& level) noexcept;
        ~LevelPanel() = default;

        void setLevel(Projects::Level& level) noexcept;

        EXAGE_DELETE_COPY(LevelPanel);
        EXAGE_DEFAULT_MOVE(LevelPanel);

        void handleFonts() noexcept;
        void run(Graphics::CommandBuffer& commandBuffer, float deltaTime) noexcept;

      private:
        Graphics::Context* _context;
        GUI::ImGui::FontManager* _fontManager;
        Projects::Level* _level;
        glm::uvec2 _viewportExtent;

        std::shared_ptr<Graphics::Texture> _testTexture;  // TODO: remove
        std::shared_ptr<Graphics::Sampler> _sampler;
        GUI::ImGui::Texture _imTexture;

        float _dpiScale = 1.0F;

        ImFont* _font;
    };
}  // namespace exitor