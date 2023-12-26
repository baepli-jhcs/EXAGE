#include <array>
#include <filesystem>

#include "ContentBrowser.h"

#include "exage/Projects/Level.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/utils/string.h"
#include "imgui.h"
#include "utils/files.h"

namespace exitor
{
    constexpr static const char* ERROR_ID = "Asset Recognition Error";
    constexpr static const char* UNKNOWN_ASSET_TYPE = "Asset Type Error";

    void ContentBrowser::render(const std::filesystem::path& baseDirectory,
                                const exage::Projects::Project& project) noexcept
    {
        std::filesystem::path truePath = getTruePath(_currentPath, baseDirectory);

        if (!std::filesystem::exists(truePath))
        {
            _currentPath = {};
            truePath = getTruePath(_currentPath, baseDirectory);
        }

        ImGui::Begin("Content Browser");

        if (!_currentPath.empty())
        {
            if (ImGui::Button("Back"))
            {
                _currentPath = _currentPath.substr(0, _currentPath.size() - 1);
                _currentPath = _currentPath.substr(0, _currentPath.find_last_of('/') + 1);
            }
        }

        // Asset recognition
        if ((_callbacks != nullptr) && ImGui::Button("Recognize Asset"))
        {
            std::array<std::string_view, 4> extensions = {
                "*.exmesh", "*.exmat", "*.exlevel", "*.extex"};

            _fileDialog.open("Recognize Asset",
                             exage::fromU8string(truePath.u8string()),
                             extensions,
                             "EXAGE Files");
        }

        assetRecognitionResults(baseDirectory);

        for (const auto& entry : std::filesystem::directory_iterator(truePath))
        {
            eachDirectoryEntry(entry, baseDirectory, project);
        }

        if (ImGui::BeginPopupModal(ERROR_ID, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Error: asset is not in project directory or subdirectory");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal(UNKNOWN_ASSET_TYPE, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Error: unknown asset type");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::End();

        // Error popu
    }

    void ContentBrowser::assetRecognitionResults(
        const std::filesystem::path& baseDirectory) noexcept
    {
        if (!_fileDialog.isReady())
        {
            return;
        }

        std::string path = _fileDialog.getResult();

        if (path.empty())
        {
            return;
        }

        std::filesystem::path fsPath = std::filesystem::u8path(path);

        _fileDialog.clear();

        // Ensure that asset is in project directory or subdirectory or subsubdirectory
        std::filesystem::path canonicalPath = std::filesystem::canonical(fsPath);
        std::filesystem::path canonicalProjectDirectory = std::filesystem::canonical(baseDirectory);

        // Make them both strings
        std::string canonicalPathString = exage::fromU8string(canonicalPath.u8string());
        std::string canonicalProjectDirectoryString =
            exage::fromU8string(canonicalProjectDirectory.u8string());

        // Check if canonicalPathString starts with canonicalProjectDirectoryString
        bool isPathInProjectDirectory =
            canonicalPathString.starts_with(canonicalProjectDirectoryString);

        if (!isPathInProjectDirectory)
        {
            ImGui::OpenPopup(ERROR_ID);
        }

        else if (_callbacks != nullptr)
        {
            std::string const extension = exage::fromU8string(fsPath.extension().u8string());

            if (extension == ".exmesh")
            {
                _callbacks->recognizeMesh(path);
            }
            else if (extension == ".exmat")
            {
                _callbacks->recognizeMaterial(path);
            }
            else if (extension == ".extex")
            {
                _callbacks->recognizeTexture(path);
            }
            else if (extension == ".exlevel")
            {
                _callbacks->recognizeLevel(path);
            }
            else
            {
                ImGui::OpenPopup(UNKNOWN_ASSET_TYPE);
            }
        }
    }

    void ContentBrowser::eachDirectoryEntry(const std::filesystem::directory_entry& entry,
                                            const std::filesystem::path& baseDirectory,
                                            const exage::Projects::Project& project) noexcept
    {
        const std::filesystem::path& path = entry.path();
        std::string filename = exage::fromU8string(path.filename().u8string());

        std::string filePath = _currentPath + filename;

        bool const isDirectory = entry.is_directory();

        if (isDirectory)
        {
            if (ImGui::Selectable(filename.c_str()))
            {
                _currentPath += filename + "/";
            }
        }
        else
        {
            // If in project.levelPaths, project.texturePaths, project.meshPaths, or
            // project.materialPaths then create a selectable

            auto isPathInVector = [&filePath](const auto& vector)
            {
                for (const auto& path : vector)
                {
                    if (path == filePath)
                    {
                        return true;
                    }
                }

                return false;
            };

            if (isPathInVector(project.levelPaths) || isPathInVector(project.texturePaths)
                || isPathInVector(project.meshPaths) || isPathInVector(project.materialPaths))
            {
                if (ImGui::Selectable(filename.c_str()))
                {
                    _selectedFile = filePath;
                }
            }
        }
    }
}  // namespace exitor