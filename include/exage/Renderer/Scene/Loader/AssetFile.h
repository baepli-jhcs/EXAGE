#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <ostream>

#include <tl/expected.hpp>

#include "exage/Core/Core.h"
#include "exage/Core/Errors.h"
#include "nlohmann/json.hpp"

namespace exage::Renderer
{
    struct AssetFile
    {
        std::string json;
        std::vector<char> binary;
    };

    inline void saveAssetFile(std::ostream& stream, const AssetFile& assetFile)
    {
        uint64_t jsonSize = assetFile.json.size();
        stream.write(reinterpret_cast<const char*>(&jsonSize), sizeof(jsonSize));
        stream.write(assetFile.json.data(), static_cast<std::streamsize>(jsonSize));
        uint64_t binarySize = assetFile.binary.size();
        stream.write(reinterpret_cast<const char*>(&binarySize), sizeof(binarySize));
        stream.write(assetFile.binary.data(), static_cast<std::streamsize>(binarySize));
    }

    [[nodiscard]] inline auto saveAssetFile(const std::filesystem::path& path,
                                            const AssetFile& assetFile) -> tl::expected<void, Error>
    {
        std::ofstream stream(path, std::ios::binary);

        if (!stream.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }
        saveAssetFile(stream, assetFile);
        return {};
    }

    [[nodiscard]] inline auto loadAssetFile(std::ifstream& stream) -> AssetFile
    {
        AssetFile assetFile;
        uint64_t jsonSize;
        stream.read(reinterpret_cast<char*>(&jsonSize), sizeof(jsonSize));
        assetFile.json.resize(static_cast<size_t>(jsonSize));
        stream.read(assetFile.json.data(), static_cast<std::streamsize>(jsonSize));
        uint64_t binarySize;
        stream.read(reinterpret_cast<char*>(&binarySize), sizeof(binarySize));
        assetFile.binary.resize(static_cast<size_t>(binarySize));
        stream.read(assetFile.binary.data(), static_cast<std::streamsize>(binarySize));
        return assetFile;
    }

    [[nodiscard]] inline auto loadAssetFile(const std::filesystem::path& path)
        -> tl::expected<AssetFile, Error>
    {
        std::ifstream stream(path, std::ios::binary);

        if (!stream.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }

        return loadAssetFile(stream);
    }

}  // namespace exage::Renderer