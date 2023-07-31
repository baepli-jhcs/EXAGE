#pragma once

#include <filesystem>

#include "exage/Projects/Project.h"
#include "exage/utils/classes.h"

namespace exitor
{
    class ContentBrowser
    {
      public:
        ContentBrowser() noexcept = default;
        ~ContentBrowser() = default;

        EXAGE_DEFAULT_COPY(ContentBrowser);
        EXAGE_DEFAULT_MOVE(ContentBrowser);

        void setProject(const std::filesystem::path& projectDirectory,
                        const exage::Projects::Project& project) noexcept;

        void render() noexcept;

      private:
        std::filesystem::path _projectDirectory;
        exage::Projects::Project _project;

        std::string _currentPath;

        std::string _selectedFile;
    };
}  // namespace exitor