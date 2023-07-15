#pragma once

#include <filesystem>
#include <string>

#include "exage/Core/Core.h"

namespace exage::Projects
{
    struct Project
    {
        std::string name;

        std::string defaultLevelPath;
        std::vector<std::string> levelPaths;

        std::vector<std::string> texturePaths;
        std::vector<std::string> meshPaths;
        std::vector<std::string> materialPaths;

        // TODO: add more configuration options, including scripts

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(name, defaultLevelPath, levelPaths, texturePaths, meshPaths, materialPaths);
        }
    };
}  // namespace exage::Projects