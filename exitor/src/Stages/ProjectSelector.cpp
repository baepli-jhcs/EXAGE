#include <fstream>

#include "ProjectSelector.h"

#include <cereal/types/chrono.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>
#include <exage/utils/serialization.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Stages/ProjectSelector.h"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "exage/Filesystem/Directories.h"
#include "exage/Projects/Project.h"
#include "exage/utils/imgui.h"
#include "exage/utils/string.h"
#include "imgui.h"
#include "tinyfiledialogs.h"
#include "utils/files.h"

namespace exitor
{

    ProjectSelector::ProjectSelector(Renderer::FontManager& fontManager) noexcept
        : _fontManager(fontManager)
    {
        getRecentProjects();

        _headerFont = _fontManager.get().getFont("Source Sans Pro Bold", 30.0F);
        _recentProjectsFont = _fontManager.get().getFont("Source Sans Pro Bold", 20.0F);
    }

    auto ProjectSelector::run() noexcept -> std::optional<ProjectReturn>
    {
        std::optional<Projects::Project> project = std::nullopt;
        std::filesystem::path projectPath;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {20.0f, 20.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

        bool open = true;
        ImGui::Begin("Project Selector", &open, windowFlags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::PushFont(_headerFont);
        ImGui::Text("Create or Select a Project");
        ImGui::PopFont();
        ImGui::Separator();

        exage::ImGuiUtils::Spacing(3);

        ImGui::InputTextWithHint("Project Name", "Enter a name for your project", &_projectName);
        ImGui::SameLine();
        ImGui::InputTextWithHint("Project Path", "Enter a path for your project", &_projectPath);
        ImGui::SameLine();
        if (ImGui::Button("Browse"))
        {
            // _fileDialog.OpenDialog(
            //     "Choose Project Directory", "Choose a project directory", nullptr, ".");

            // char* projectPath = tinyfd_selectFolderDialog("Choose Project Directory", nullptr);
            // if (projectPath != nullptr)
            // {
            //     _projectPath = projectPath;
            // }

            _folderDialog.open("Choose Project Directory", {});
        }

        if (_folderDialog.isReady())
        {
            std::string result = _folderDialog.getResult();
            if (!result.empty())
            {
                _projectPath = result;
            }
        }

        // if (_fileDialog.Display("Choose Project Directory", ImGuiWindowFlags_NoCollapse))
        // {
        //     if (_fileDialog.IsOk())
        //     {
        //         _projectPath = _fileDialog.GetFilePathName();
        //     }

        //     _fileDialog.Close();
        // }

        // if (_folderDialog && _folderDialog->ready())
        // {
        //     std::string result = _folderDialog->result();
        //     if (!result.empty())
        //     {
        //         _projectPath = result;
        //     }

        //     _folderDialog = std::nullopt;
        // }

        exage::ImGuiUtils::Spacing(3);

        // New project
        if (ImGui::Button("New Project"))
        {
            // Remove spaces from project name
            _projectName.erase(std::remove_if(_projectName.begin(),
                                              _projectName.end(),
                                              [](char c) { return std::isspace(c); }),
                               _projectName.end());

            project = createProject(_projectPath);
            projectPath = _projectPath;
        }

        exage::ImGuiUtils::Spacing(3);
        ImGui::Separator();

        // Recent projects
        if (!_recentProjects.empty())
        {
            ImGui::PushFont(_recentProjectsFont);

            exage::ImGuiUtils::Spacing(3);

            ImGui::Text("Recent Projects");

            ImGui::Spacing();
            ImGui::Separator();

            ImGui::PopFont();

            for (const auto& recentProject : _recentProjects)
            {
                exage::ImGuiUtils::Spacing(3);

                if (ImGui::Button(recentProject.name.c_str()))
                {
                    project = loadProject(std::filesystem::u8path(recentProject.path));
                    projectPath = recentProject.path;
                }

                ImGui::SameLine();
                ImGui::Text("%s", recentProject.path.c_str());

                exage::ImGuiUtils::Spacing(3);
            }

            ImGui::Separator();
        }

        ImGuiUtils::Spacing(5);

        // Existing project
        if (ImGui::Button("Open Project"))
        {
            // _fileDialog.OpenDialog("Choose a project file", "Choose", ".exproj", ".");

            std::array<std::string_view, 1> filters = {"*.exproj"};
            _fileDialog.open("Choose a project file", "", filters, "Project");
        }

        if (_fileDialog.isReady())
        {
            std::string result = _fileDialog.getResult();
            if (!result.empty())
            {
                projectPath = result;
                project = loadProject(projectPath);
            }

            _fileDialog.clear();
        }

        // if (_fileDialog.Display("Choose a project file", ImGuiWindowFlags_NoCollapse))
        // {
        //     if (_fileDialog.IsOk())
        //     {
        //         projectPath = _fileDialog.GetFilePathName();
        //         project = loadProject(projectPath);
        //     }

        //     _fileDialog.Close();
        // }

        ImGui::End();

        if (project)
        {
            return ProjectReturn {*project, projectPath, projectPath.parent_path()};
        }

        return std::nullopt;
    }

    void ProjectSelector::getRecentProjects() noexcept
    {
        auto projectFile = getUserDataDirectory();
        projectFile /= "projects.list";

        if (!std::filesystem::exists(projectFile))
        {
            return;
        }

        std::ifstream file(projectFile, std::ios::binary);
        if (!file.is_open())
        {
            return;
        }

        try
        {
            cereal::BinaryInputArchive archive(file);
            archive(_recentProjects);

            file.close();
        }
        catch (const std::exception& e)
        {
            return;
        }

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

    auto ProjectSelector::loadProject(const std::filesystem::path& path) noexcept
        -> std::optional<Projects::Project>
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return {};
        }

        Projects::Project project;

        try
        {
            cereal::BinaryInputArchive archive(file);
            archive(project);

            file.close();
        }
        catch (const std::exception& e)
        {
            return {};
        }

        // Update last opened
        addOrUpdateRecentProject(path, project.name);

        return project;
    }

    auto ProjectSelector::createProject(const std::filesystem::path& path) noexcept
        -> std::optional<Projects::Project>
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
        {
            return {};
        }

        if (_projectName.empty())
        {
            return {};
        }

        std::filesystem::path projectDir = path / _projectName;
        std::filesystem::create_directories(projectDir);

        // Copy from template
        std::filesystem::path templateDir = Filesystem::getEngineAssetDirectory() / "template";
        std::filesystem::copy(templateDir,
                              projectDir,
                              std::filesystem::copy_options::recursive
                                  | std::filesystem::copy_options::overwrite_existing);

        // Replace EXAGE_PROJECT_NAME in CMakeLists.txt with project name and EXAGE_PATH with engine
        std::filesystem::path cmakeLists = projectDir / "CMakeLists.txt";

        std::ifstream file(cmakeLists);
        if (!file.is_open())
        {
            return {};
        }

        std::string cmakeListsContents {std::istreambuf_iterator<char>(file),
                                        std::istreambuf_iterator<char>()};
        file.close();

        cmakeListsContents.replace(cmakeListsContents.find("EXAGE_PROJECT_NAME"),
                                   std::string_view("EXAGE_PROJECT_NAME").length(),
                                   _projectName);

        cmakeListsContents.replace(
            cmakeListsContents.find("EXAGE_PATH"),
            std::string_view("EXAGE_PATH").length(),
            fromU8string(std::filesystem::absolute(std::filesystem::current_path()).u8string()));

        std::ofstream saveFile(cmakeLists);
        if (saveFile.is_open())
        {
            saveFile << cmakeListsContents;
            saveFile.close();
        }

        // Create project file
        Projects::Project project;
        project.name = _projectName;
        project.defaultLevelPath = "assets/levels/default.exlevel";
        project.levelPaths.insert(project.defaultLevelPath);

        std::filesystem::path projectFile = projectDir / (_projectName + ".exproj");

        std::ofstream projectSaveFile(projectFile, std::ios::binary);
        if (projectSaveFile.is_open())
        {
            cereal::BinaryOutputArchive archive(projectSaveFile);
            archive(project);

            projectSaveFile.close();
        }
        else  // Failed to create project file
        {
            return {};
        }

        // Update last opened
        addOrUpdateRecentProject(projectFile, project.name);

        return project;
    }

    void ProjectSelector::addOrUpdateRecentProject(const std::filesystem::path& path,
                                                   const std::string& name) noexcept
    {
        auto it = std::find_if(_recentProjects.begin(),
                               _recentProjects.end(),
                               [&path](const auto& p) { return p.path == path; });

        if (it != _recentProjects.end())
        {
            it->lastOpened = std::chrono::system_clock::now();
        }
        else
        {
            ProjectMetadata metadata;
            metadata.name = name;
            metadata.path = fromU8string(path.u8string());
            metadata.lastOpened = std::chrono::system_clock::now();

            _recentProjects.push_back(metadata);
        }

        // Save recent projects
        auto projectFile = getUserDataDirectory();
        std::filesystem::create_directories(projectFile);
        projectFile /= "projects.list";

        std::ofstream saveFile(projectFile, std::ios::binary);
        if (saveFile.is_open())
        {
            cereal::BinaryOutputArchive archive(saveFile);
            archive(_recentProjects);

            saveFile.close();
        }
    }

}  // namespace exitor