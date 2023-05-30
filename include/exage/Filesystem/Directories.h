#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "exage/Core/Core.h"

namespace exage::Filesystem
{
    void setEngineAssetDirectory(std::filesystem::path path) noexcept;
    void setEngineShaderDirectory(std::filesystem::path path) noexcept;
    void setEngineShaderCacheDirectory(std::filesystem::path path) noexcept;

    [[nodiscard]] auto getEngineAssetDirectory() noexcept -> const std::filesystem::path&;
    [[nodiscard]] auto getEngineShaderDirectory() noexcept -> const std::filesystem::path&;
    [[nodiscard]] auto getEngineShaderCacheDirectory() noexcept -> const std::filesystem::path&;

    struct PathHash
    {
        // Hash for std::filesystem::path
        [[nodiscard]] auto operator()(const std::filesystem::path& path) const noexcept
            -> std::size_t
        {
            return std::filesystem::hash_value(path);
        }
    };
}  // namespace exage::Filesystem
