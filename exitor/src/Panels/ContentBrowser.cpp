#include <filesystem>

#include "ContentBrowser.h"

#include <imgui.h>

#include "exage/utils/string.h"
#include "utils/files.h"

namespace exitor
{
    void ContentBrowser::setProject(const std::filesystem::path& projectDirectory,
                                    const exage::Projects::Project& project) noexcept
    {
        _projectDirectory = projectDirectory;
        _project = project;

        _currentPath = "";
    }

    void ContentBrowser::render() noexcept
    {
        std::filesystem::path truePath = getTruePath(_currentPath, _projectDirectory);

        if (!std::filesystem::exists(truePath))
        {
            _currentPath = "project/";
            truePath = getTruePath(_currentPath, _projectDirectory);
        }

        ImGui::Begin("Content Browser");

        auto forEachDirectoryEntry = [this](const auto& entry)
        {
            const std::filesystem::path& path = entry.path();
            std::string filename = exage::fromU8string(path.filename().u8string());

            bool isDirectory = entry.is_directory();
        };
    }
}  // namespace exitor