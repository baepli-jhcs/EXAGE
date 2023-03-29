#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "exage/Core/Core.h"

namespace exage::Filesystem
{
    EXAGE_EXPORT void setEngineAssetDirectory(std::filesystem::path path) noexcept;
    EXAGE_EXPORT void setEngineShaderDirectory(std::filesystem::path path) noexcept;
    EXAGE_EXPORT void setEngineShaderCacheDirectory(std::filesystem::path path) noexcept;

    [[nodiscard]] EXAGE_EXPORT auto getEngineAssetDirectory() noexcept -> const std::filesystem::path&;
    [[nodiscard]] EXAGE_EXPORT auto getEngineShaderDirectory() noexcept -> const std::filesystem::path&;
	[[nodiscard]] EXAGE_EXPORT auto getEngineShaderCacheDirectory() noexcept -> const std::filesystem::path&;
}  // namespace exage::Filesystem
