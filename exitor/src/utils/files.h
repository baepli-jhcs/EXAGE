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

    inline auto appendFolder(const std::string& path, const std::string& folder) noexcept
        -> std::string
    {
        if (path.empty())
        {
            return folder + "/";
        }

        if (path.ends_with("/"))
        {
            return path + folder + "/";
        }

        return path + "/" + folder + "/";
    }
}  // namespace exitor