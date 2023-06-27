#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <tl/expected.hpp>

#include "exage/Core/Core.h"
#include "exage/Scene/Scene.h"

namespace exage::Projects
{
    using ComponentData = std::unordered_map<uint32_t, std::string>;

    struct Level
    {
        std::filesystem::path path;

        std::vector<std::filesystem::path> texturePaths;
        std::vector<std::filesystem::path> materialPaths;
        std::vector<std::filesystem::path> meshPaths;

        uint32_t entityCount;
        std::unordered_map<std::string, ComponentData> componentData;
    };

    struct DeserializedLevel
    {
        std::filesystem::path path;

        std::vector<std::filesystem::path> texturePaths;
        std::vector<std::filesystem::path> materialPaths;
        std::vector<std::filesystem::path> meshPaths;

        Scene scene;
    };

    [[nodiscard]] auto loadLevel(const std::filesystem::path& path) noexcept -> Level;
    void saveLevel(const std::filesystem::path& path, const Level& level) noexcept;

    [[nodiscard]] auto deserializeLevel(Level& level) noexcept -> DeserializedLevel;
}  // namespace exage::Projects