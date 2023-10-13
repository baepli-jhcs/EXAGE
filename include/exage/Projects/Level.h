#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <tl/expected.hpp>

#include "exage/Core/Core.h"
#include "exage/Core/Errors.h"
#include "exage/Scene/Scene.h"

namespace exage::Projects
{
    using ComponentData = std::unordered_map<uint32_t, std::string>;

    struct SerializedLevel
    {
        std::string path;

        std::unordered_set<std::string> texturePaths;
        std::unordered_set<std::string> materialPaths;
        std::unordered_set<std::string> meshPaths;

        uint32_t entityCount;
        std::unordered_map<std::string, ComponentData> componentData;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(path, texturePaths, materialPaths, meshPaths, entityCount, componentData);
        }
    };

    struct Level
    {
        std::string path;

        std::unordered_set<std::string> texturePaths;
        std::unordered_set<std::string> materialPaths;
        std::unordered_set<std::string> meshPaths;

        Scene scene;
    };

    [[nodiscard]] auto loadLevel(const std::filesystem::path& path) noexcept
        -> tl::expected<SerializedLevel, Error>;
    [[nodiscard]] auto saveLevel(const std::filesystem::path& path, const SerializedLevel& level) noexcept
        -> tl::expected<void, Error>;

    [[nodiscard]] auto deserializeLevel(SerializedLevel& level) noexcept -> Level;
    [[nodiscard]] auto serializeLevel(const Level& level) noexcept -> SerializedLevel;

    constexpr std::string_view LEVEL_EXTENSION = ".exlevel";
}  // namespace exage::Projects