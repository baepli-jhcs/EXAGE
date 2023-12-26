#pragma once

#include <exage/Graphics/CommandBuffer.h>
#include <exage/Projects/Level.h>
#include <exage/Projects/Project.h>

#include "LevelEditor/Panels/LevelPanel.h"

namespace exitor
{
    using namespace exage;

    struct LevelEditorCreateInfo
    {
        Graphics::Context* context;
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

        void run(Graphics::CommandBuffer& commandBuffer, float deltaTime) noexcept;

      private:
        void menuBar() noexcept;

        Projects::Project _project;
        std::filesystem::path _projectPath;
        std::filesystem::path _projectDirectory;

        Projects::Level _level;
        LevelPanel _levelPanel;
    };
}  // namespace exitor