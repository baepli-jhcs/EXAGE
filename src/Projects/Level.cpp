#include <fstream>
#include <string>
#include <string_view>

#include "exage/Projects/Level.h"

#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

#include "exage/Core/Core.h"
#include "exage/Projects/Serialization.h"

namespace exage::Projects
{
    auto loadLevel(const std::filesystem::path& path) noexcept -> Level
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);

        std::string jsonStr;
        jsonStr.reserve(std::filesystem::file_size(path));
        jsonStr.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        nlohmann::json json = nlohmann::json::parse(jsonStr);

        Level level;
        level.path = json["path"].get<std::filesystem::path>();
        level.texturePaths = json["texturePaths"].get<std::vector<std::filesystem::path>>();
        level.materialPaths = json["materialPaths"].get<std::vector<std::filesystem::path>>();
        level.meshPaths = json["meshPaths"].get<std::vector<std::filesystem::path>>();
        level.entityCount = json["entityCount"].get<uint32_t>();
        level.componentData = std::unordered_map<std::string, ComponentData>();

        for (const auto& componentDataJson : json["componentData"])
        {
            ComponentData componentData;
            for (const auto& [entityId, data] : componentDataJson["data"].items())
            {
                componentData[std::stoul(entityId)] = data.get<std::string>();
            }

            level.componentData[componentDataJson["typeName"].get<std::string>()] = componentData;
        }

        return level;
    }

    void saveLevel(const std::filesystem::path& path, const Level& level) noexcept
    {
        nlohmann::json json;
        json["path"] = level.path;
        json["texturePaths"] = level.texturePaths;
        json["materialPaths"] = level.materialPaths;
        json["meshPaths"] = level.meshPaths;
        json["entityCount"] = level.entityCount;
        json["componentData"] = nlohmann::json::array();

        for (const auto& [key, data] : level.componentData)
        {
            nlohmann::json componentDataJson;
            componentDataJson["typeName"] = key;
            componentDataJson["data"] = nlohmann::json::object();

            for (const auto& [entityId, data] : data)
            {
                componentDataJson["data"][std::to_string(entityId)] = data;
            }

            json["componentData"].push_back(componentDataJson);
        }

        std::ofstream file(path, std::ios::out | std::ios::binary);
        file << json.dump(4);
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

}  // namespace exage::Projects