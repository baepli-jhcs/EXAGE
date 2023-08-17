#pragma once

#include <filesystem>

#include <exage/Filesystem/Directories.h>

namespace exitor
{
    inline auto getUserDataDirectory() noexcept -> std::filesystem::path
    {
        return exage::Filesystem::getApplicationDataPath() / "EXitor";
    }

    inline auto getTruePath(const std ::string& path,
                            const std::filesystem::path& rootDirectory) noexcept
        -> std::filesystem::path
    {
        return rootDirectory / std::filesystem::u8path(path);
    }
}  // namespace exitor