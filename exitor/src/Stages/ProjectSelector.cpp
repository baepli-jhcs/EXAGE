#include <fstream>

#include "ProjectSelector.h"

#include <cereal/archives/json.hpp>

#include "utils/files.h"

namespace exitor
{

    ProjectSelector::ProjectSelector() noexcept
    {
        getRecentProjects();
    }

    auto ProjectSelector::run() noexcept -> std::optional<Projects::Project>
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

        bool open = true;
        ImGui::Begin("Project Selector", &open, windowFlags);
        ImGui::PopStyleVar();

        // Recent projects
        if (!_recentProjects.empty())
        {
            ImGui::Text("Recent Projects");
            ImGui::Separator();

            for (const auto& project : _recentProjects)
            {
                if (ImGui::Button(project.name.c_str()))
                {
                    return Projects::Project(project.path);
                }
            }

            ImGui::Separator();
        }
    }

    void ProjectSelector::getRecentProjects() noexcept
    {
        auto projectFile = getUserDataDirectory();
        projectFile /= "projects.json";

        std::ifstream file(projectFile);
        if (!file.is_open())
        {
            return;
        }

        cereal::JSONInputArchive archive(file);
        archive(_recentProjects);

        file.close();

        // Remove non-existing projects
        for (auto it = _recentProjects.begin(); it != _recentProjects.end();)
        {
            if (!std::filesystem::exists(it->path))
            {
                it = _recentProjects.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Sort by last opened
        std::sort(_recentProjects.begin(),
                  _recentProjects.end(),
                  [](const auto& a, const auto& b) { return a.lastOpened > b.lastOpened; });
    }

}  // namespace exitor