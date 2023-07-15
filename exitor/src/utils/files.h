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
        if (path.starts_with("project/"))
        {
            return rootDirectory / std::filesystem::u8path(path.substr(8));
        }

        // engine/
        return std::filesystem::u8path(path.substr(7));
    }
}  // namespace exitor