#include "exage/Filesystem/Directories.h"

namespace exage::Filesystem
{
    namespace
    {
        std::filesystem::path engineAssetDirectory = "assets/exage";
        std::filesystem::path engineShaderDirectory = "shaders/exage";
        std::filesystem::path engineShaderCacheDirectory = "cache/shaders/exage";

        std::filesystem::path applicationDataPath;
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

    auto getApplicationDataPath() noexcept -> const std::filesystem::path&
    {
        if (!applicationDataPath.empty())
        {
            return applicationDataPath;
        }

#ifdef EXAGE_WINDOWS
        applicationDataPath = std::getenv("LOCALAPPDATA");

#elif defined(EXAGE_LINUX)
        std::filesystem::path home = std::getenv("HOME");
        applicationDataPath = home / ".config";
#elif defined(EXAGE_MACOS)
        std::filesystem::path home = std::getenv("HOME");
        applicationDataPath = home / "Library/Application Support";
#else
        applicationDataPath = ".";
#endif

        return applicationDataPath;
    }

}  // namespace exage::Filesystem
