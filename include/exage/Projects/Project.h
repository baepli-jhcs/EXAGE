#pragma once

#include <filesystem>
#include <string>

#include "exage/Core/Core.h"

namespace exage::Projects
{
    struct Project
    {
        std::string name;

        std::filesystem::path defaultLevelPath;
        std::vector<std::filesystem::path> levelPaths;

        // TODO: add more configuration options, including scripts
    };
}  // namespace exage::Projects