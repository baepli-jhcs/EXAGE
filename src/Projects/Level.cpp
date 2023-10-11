#include <fstream>
#include <string>
#include <string_view>

#include "exage/Projects/Level.h"

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>
#include <exage/utils/serialization.h>
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

#include "cereal/archives/binary.hpp"
#include "exage/Core/Core.h"
#include "exage/Projects/Serialization.h"

namespace exage::Projects
{
    auto loadLevel(const std::filesystem::path& path) noexcept -> tl::expected<Level, Error>
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);

        if (!file.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }

        Level level;

        try
        {
            cereal::BinaryInputArchive archive(file);
            archive(level);
        }
        catch (const std::exception& e)
        {
            return tl::make_unexpected(Errors::DeserializationFailed {});
        }

        // std::string jsonStr;
        // jsonStr.reserve(std::filesystem::file_size(path));
        // jsonStr.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        // nlohmann::json json = nlohmann::json::parse(jsonStr);

        // Level level;
        // level.path = json["path"].get<std::filesystem::path>();
        // level.texturePaths = json["texturePaths"].get<std::vector<std::filesystem::path>>();
        // level.materialPaths = json["materialPaths"].get<std::vector<std::filesystem::path>>();
        // level.meshPaths = json["meshPaths"].get<std::vector<std::filesystem::path>>();
        // level.entityCount = json["entityCount"].get<uint32_t>();
        // level.componentData = std::unordered_map<std::string, ComponentData>();

        // for (const auto& componentDataJson : json["componentData"])
        // {
        //     ComponentData componentData;
        //     for (const auto& [entityId, data] : componentDataJson["data"].items())
        //     {
        //         componentData[std::stoul(entityId)] = data.get<std::string>();
        //     }

        //     level.componentData[componentDataJson["typeName"].get<std::string>()] =
        //     componentData;
        // }

        return level;
    }

    auto saveLevel(const std::filesystem::path& path, const Level& level) noexcept
        -> tl::expected<void, Error>
    {
        // nlohmann::json json;
        // json["path"] = level.path;
        // json["texturePaths"] = level.texturePaths;
        // json["materialPaths"] = level.materialPaths;
        // json["meshPaths"] = level.meshPaths;
        // json["entityCount"] = level.entityCount;
        // json["componentData"] = nlohmann::json::array();

        // for (const auto& [key, data] : level.componentData)
        // {
        //     nlohmann::json componentDataJson;
        //     componentDataJson["typeName"] = key;
        //     componentDataJson["data"] = nlohmann::json::object();

        //     for (const auto& [entityId, data] : data)
        //     {
        //         componentDataJson["data"][std::to_string(entityId)] = data;
        //     }

        //     json["componentData"].push_back(componentDataJson);
        // }

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }
        // file << json.dump(4);

        try
        {
            cereal::BinaryOutputArchive archive(file);
            archive(level);
        }
        catch (const std::exception& e)
        {
            return tl::make_unexpected(Errors::SerializationFailed {});
        }

        return {};
    }

    auto deserializeLevel(Level& level) noexcept -> DeserializedLevel
    {
        DeserializedLevel deserializedLevel;
        deserializedLevel.path = level.path;
        deserializedLevel.texturePaths = level.texturePaths;
        deserializedLevel.materialPaths = level.materialPaths;
        deserializedLevel.meshPaths = level.meshPaths;
        deserializedLevel.scene = loadScene(level.entityCount, level.componentData);

        return deserializedLevel;
    }

    auto serializeLevel(const DeserializedLevel& level) noexcept -> Level
    {
        Level serializedLevel;
        serializedLevel.path = level.path;
        serializedLevel.texturePaths = level.texturePaths;
        serializedLevel.materialPaths = level.materialPaths;
        serializedLevel.meshPaths = level.meshPaths;

        auto [entityCount, data] = serializeScene(level.scene);

        serializedLevel.entityCount = entityCount;
        serializedLevel.componentData = std::move(data);

        return serializedLevel;
    }

}  // namespace exage::Projects