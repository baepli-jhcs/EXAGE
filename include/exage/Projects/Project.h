#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>

#include <tl/expected.hpp>

#include "exage/Core/Core.h"
#include "exage/Core/Errors.h"

namespace exage::Projects
{
    struct Project
    {
        std::string name;

        std::string defaultLevelPath;
        std::unordered_set<std::string> levelPaths;

        std::unordered_set<std::string> texturePaths;
        std::unordered_set<std::string> meshPaths;
        std::unordered_set<std::string> materialPaths;

        // TODO: add more configuration options, including scripts

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(name, defaultLevelPath, levelPaths, texturePaths, meshPaths, materialPaths);
        }
    };

    [[nodiscard]] auto loadProject(const std::filesystem::path& path) noexcept
        -> tl::expected<Project, Error>;
    [[nodiscard]] auto saveProject(const std::filesystem::path& path,
                                   const Project& project) noexcept -> tl::expected<void, Error>;

}  // namespace exage::Projects