#pragma once

#include <filesystem>

#include <exage/Filesystem/Directories.h>

namespace exitor
{
    inline auto getUserDataDirectory() noexcept -> std::filesystem::path
    {
        return exage::Filesystem::getApplicationDataPath() / "EXitor";
    }
}  // namespace exitor