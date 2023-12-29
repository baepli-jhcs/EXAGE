#pragma once

#include <exage/GUI/Fonts.h>
#include <exage/Graphics/CommandBuffer.h>
#include <exage/Projects/Level.h>
#include <exage/Projects/Project.h>

#include "LevelEditor/Panels/ComponentEditor.h"
#include "LevelEditor/Panels/ComponentList.h"
#include "LevelEditor/Panels/Hierarchy.h"
#include "LevelEditor/Panels/LevelPanel.h"
#include "imgui.h"

namespace exitor
{
    using namespace exage;

    struct LevelEditorCreateInfo
    {
        Graphics::Context* context;
        GUI::ImGui::FontManager* fontManager;
        Projects::Project project;
        std::filesystem::path projectPath;
        std::filesystem::path projectDirectory;
    };

    class LevelEditor
    {
      public:
        explicit LevelEditor(const LevelEditorCreateInfo& createInfo) noexcept;
        ~LevelEditor() = default;

        EXAGE_DELETE_COPY(LevelEditor);
        EXAGE_DEFAULT_MOVE(LevelEditor);

        void handleFonts() noexcept;
        void run(Graphics::CommandBuffer& commandBuffer, float deltaTime) noexcept;

      private:
        void menuBar() noexcept;

        GUI::ImGui::FontManager* _fontManager;

        Projects::Project _project;
        std::filesystem::path _projectPath;
        std::filesystem::path _projectDirectory;

        Projects::Level _level;
        LevelPanel _levelPanel;
        HierarchyPanel _hierarchyPanel;
        ComponentList _componentList;
        ComponentEditor _componentEditor;

        ImFont* _menuBarFont;

        float _dpiScale = 1.F;
    };
}  // namespace exitor