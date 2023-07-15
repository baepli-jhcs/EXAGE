#pragma once

#include "ImGuiFileDialog.h"
#include "exage/Core/Core.h"
#include "exage/Core/Timer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "exage/Graphics/Queue.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Graphics/Utils/QueueCommand.h"
#include "exage/Projects/Level.h"
#include "exage/Projects/Project.h"
#include "exage/Renderer/Renderer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/Utils/Fonts.h"

namespace exitor
{
    using namespace exage;

    struct ProjectReturn
    {
        Projects::Project project;
        std::filesystem::path path;
        std::filesystem::path directory;
    };

    class ProjectSelector
    {
      public:
        explicit ProjectSelector(Renderer::FontManager& fontManager) noexcept;
        ~ProjectSelector() = default;

        EXAGE_DEFAULT_COPY(ProjectSelector);
        EXAGE_DEFAULT_MOVE(ProjectSelector);

        auto run() noexcept -> std::optional<ProjectReturn>;

      private:
        struct ProjectMetadata
        {
            std::string name;
            std::string path;
            std::chrono::system_clock::time_point lastOpened;

            // cereal serialization
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(name, path, lastOpened);
            }
        };

        void getRecentProjects() noexcept;
        [[nodiscard]] auto loadProject(const std::filesystem::path& path) noexcept
            -> std::optional<Projects::Project>;
        [[nodiscard]] auto createProject(const std::filesystem::path& path) noexcept
            -> std::optional<Projects::Project>;

        void addOrUpdateRecentProject(const std::filesystem::path& path,
                                      const std::string& name) noexcept;

        std::reference_wrapper<Renderer::FontManager> _fontManager;
        IGFD::FileDialog _fileDialog;
        std::vector<ProjectMetadata> _recentProjects {};

        std::string _projectName {};
        std::string _projectPath {};
    };

}  // namespace exitor
