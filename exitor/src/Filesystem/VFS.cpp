#pragma once

#include <iostream>

#include "VFS.h"

#include "exage/utils/string.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "utils/files.h"

namespace exitor::VFS
{
    void getFolders(const std::filesystem::path& basePath,
                    const exage::Projects::Project& project,
                    const std::string& path,
                    std::vector<std::string>& folders) noexcept
    {
        std::filesystem::path fullPath = getTruePath(path, basePath);

        if (!std::filesystem::exists(fullPath))
        {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(fullPath))
        {
            if (entry.is_directory())
            {
                folders.emplace_back(entry.path().filename().string());
            }
        }
    }

    void getFiles(const std::filesystem::path& basePath,
                  const exage::Projects::Project& project,
                  const std::string& path,
                  std::vector<std::string>& files) noexcept
    {
        std::filesystem::path fullPath = getTruePath(path, basePath);

        if (!std::filesystem::exists(fullPath))
        {
            return;
        }

        auto isPathInVector = [](const auto& vector, const auto& path) -> bool
        {
            for (const auto& fs : vector)
            {
                if (path == fs)
                {
                    return true;
                }
            }

            return false;
        };

        for (const auto& entry : std::filesystem::directory_iterator(fullPath))
        {
            if (entry.is_regular_file())
            {
                std::string filename = exage::fromU8string(entry.path().filename().u8string());
                std::string filePath = path + filename;

                if (isPathInVector(project.levelPaths, filePath)
                    || isPathInVector(project.texturePaths, filePath)
                    || isPathInVector(project.meshPaths, filePath)
                    || isPathInVector(project.materialPaths, filePath))
                {
                    files.emplace_back(filename);
                }
            }
        }
    }

    void getFilesAndFolders(const std::filesystem::path& basePath,
                            const exage::Projects::Project& project,
                            const std::string& path,
                            std::vector<std::string>& files,
                            std::vector<std::string>& folders) noexcept
    {
        std::filesystem::path fullPath = getTruePath(path, basePath);

        if (!std::filesystem::exists(fullPath))
        {
            return;
        }

        auto isPathInVector = [](const auto& vector, const auto& path) -> bool
        {
            for (const auto& fs : vector)
            {
                if (path == fs)
                {
                    return true;
                }
            }

            return false;
        };

        for (const auto& entry : std::filesystem::directory_iterator(fullPath))
        {
            if (entry.is_directory())
            {
                folders.emplace_back(entry.path().filename().string());
            }
            else if (entry.is_regular_file())
            {
                std::string filename = exage::fromU8string(entry.path().filename().u8string());
                std::string filePath = path + filename;

                if (isPathInVector(project.levelPaths, filePath)
                    || isPathInVector(project.texturePaths, filePath)
                    || isPathInVector(project.meshPaths, filePath)
                    || isPathInVector(project.materialPaths, filePath))
                {
                    files.emplace_back(filename);
                }
            }
        }
    }

    auto openFilePopup(const char* label,
                       const std::filesystem::path& basePath,
                       const exage::Projects::Project& project,
                       const std::string& path,
                       std::string& selectedPath) noexcept -> bool
    {
        if (!ImGui::BeginPopupModal(label, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return false;
        }

        static std::vector<std::string> files;
        static std::vector<std::string> folders;
        static std::string currentPath = path;

        if (ImGui::Button("Back"))
        {
            currentPath = currentPath.substr(0, currentPath.find_last_of('/'));
            currentPath = currentPath.substr(0, currentPath.find_last_of('/') + 1);
        }

        ImGui::SameLine();

        if (ImGui::Button("Select"))
        {
            selectedPath = currentPath;
            ImGui::CloseCurrentPopup();
            return true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            return false;
        }

        ImGui::SameLine();

        ImGui::Text("%s", currentPath.c_str());

        ImGui::Separator();

        files.clear();
        folders.clear();

        getFilesAndFolders(basePath, project, currentPath, files, folders);

        for (const auto& folder : folders)
        {
            if (ImGui::Selectable(folder.c_str()))
            {
                currentPath += folder + '/';
            }
        }

        for (const auto& file : files)
        {
            if (ImGui::Selectable(file.c_str()))
            {
                selectedPath = currentPath + file;
            }
        }

        ImGui::EndPopup();

        return false;
    }

    auto openFolderPopup(const char* label,
                         const std::filesystem::path& basePath,
                         const exage::Projects::Project& project,
                         std::string& path,
                         std::string& selectedPath) noexcept -> bool
    {
        if (!ImGui::BeginPopupModal(label, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return false;
        }

        static std::vector<std::string> folders;

        if (!path.empty() && !path.ends_with('/'))
        {
            path += '/';
        }

        if (!std::filesystem::exists(getTruePath(path, basePath)))
        {
            path.clear();
        }

        std::string& currentPath = path;

        bool returnValue = false;

        if (ImGui::Button("Back"))
        {
            currentPath = currentPath.substr(0, currentPath.find_last_of('/'));
            currentPath = currentPath.substr(0, currentPath.find_last_of('/') + 1);
        }

        ImGui::SameLine();

        if (ImGui::Button("Select"))
        {
            ImGui::CloseCurrentPopup();
            returnValue = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        ImGui::Text("%s", currentPath.c_str());

        ImGui::SameLine();

        constexpr static const char* NEW_FOLDER_ID = "New Folder";

        if (ImGui::Button("New Folder"))
        {
            ImGui::OpenPopup(NEW_FOLDER_ID);
        }

        if (ImGui::BeginPopup(NEW_FOLDER_ID, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // Create new folder
            static std::string newFolderName;

            ImGui::InputText("Folder Name", &newFolderName);

            std::string newFolderPathVFS = currentPath + newFolderName;
            std::filesystem::path newFolderPath = getTruePath(newFolderPathVFS, basePath);

            if (ImGui::Button("Create"))
            {
                if (!std::filesystem::exists(newFolderPath))
                {
                    std::filesystem::create_directory(newFolderPath);
                }

                newFolderName.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::Separator();

        folders.clear();

        getFolders(basePath, project, currentPath, folders);

        ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, /*enabled=*/true);

        for (const auto& folder : folders)
        {
            std::string folderPath = appendFolder(currentPath, folder);
            bool selected = folderPath == selectedPath;
            if (ImGui::Selectable(folder.c_str(), selected))
            {
                selectedPath = folderPath;
            }

            // Double click to select
            if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
            {
                currentPath = folderPath;
            }
        }

        ImGui::PopItemFlag();

        ImGui::EndPopup();

        return returnValue;
    }
}  // namespace exitor::VFS