#pragma once

#include <filesystem>

#include "exage/Projects/Project.h"

namespace exitor::VFS  // virtual filesystem
{
    void getFolders(const std::filesystem::path& basePath,
                    const exage::Projects::Project& project,
                    const std::string& path,
                    std::vector<std::string>& folders) noexcept;

    void getFiles(const std::filesystem::path& basePath,
                  const exage::Projects::Project& project,
                  const std::string& path,
                  std::vector<std::string>& files) noexcept;

    void getFilesAndFolders(const std::filesystem::path& basePath,
                            const exage::Projects::Project& project,
                            const std::string& path,
                            std::vector<std::string>& files,
                            std::vector<std::string>& folders) noexcept;

    /* Returns true if selection is made */
    auto openFilePopup(const char* label,
                       const std::filesystem::path& basePath,
                       const exage::Projects::Project& project,
                       const std::string& path,
                       std::string& selectedPath) noexcept -> bool;

    auto openFolderPopup(const char* label,
                         const std::filesystem::path& basePath,
                         const exage::Projects::Project& project,
                         std::string& path,
                         std::string& selectedPath) noexcept -> bool;

}  // namespace exitor::VFS