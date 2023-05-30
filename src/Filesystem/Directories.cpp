﻿#include "exage/Filesystem/Directories.h"

namespace exage::Filesystem
{
    namespace
    {
        std::filesystem::path engineAssetDirectory = "assets/exage";
        std::filesystem::path engineShaderDirectory = "shaders/exage";
        std::filesystem::path engineShaderCacheDirectory = "cache/shaders/exage";
    }  // namespace

    void setEngineAssetDirectory(std::filesystem::path path) noexcept
    {
        engineAssetDirectory = std::move(path);
    }

    void setEngineShaderDirectory(std::filesystem::path path) noexcept
    {
        engineShaderDirectory = std::move(path);
    }

    void setEngineShaderCacheDirectory(std::filesystem::path path) noexcept
    {
        engineShaderCacheDirectory = std::move(path);
    }

    auto getEngineAssetDirectory() noexcept -> const std::filesystem::path&
    {
        return engineAssetDirectory;
    }

    auto getEngineShaderDirectory() noexcept -> const std::filesystem::path&
    {
        return engineShaderDirectory;
    }

    auto getEngineShaderCacheDirectory() noexcept -> const std::filesystem::path&
    {
        return engineShaderCacheDirectory;
    }
}  // namespace exage::Filesystem
