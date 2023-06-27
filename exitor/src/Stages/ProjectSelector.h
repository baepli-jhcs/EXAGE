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

namespace exitor
{
    using namespace exage;

    class ProjectSelector
    {
      public:
        ProjectSelector() noexcept;
        ~ProjectSelector();

        EXAGE_DEFAULT_COPY(ProjectSelector);
        EXAGE_DEFAULT_MOVE(ProjectSelector);

        auto run() noexcept -> std::optional<Projects::Project>;

      private:
        struct ProjectMetadata
        {
            std::string name;
            std::string path;
            std::chrono::system_clock::time_point lastOpened;

            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(name, path, lastOpened);
            }
        };

        void getRecentProjects() noexcept;

        ImGuiFileDialog _fileDialog;
        std::vector<ProjectMetadata> _recentProjects {};
    };

}  // namespace exitor
