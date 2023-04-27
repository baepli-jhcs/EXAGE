#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Loader/Errors.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/Scene/Scene.h"
#include "tl/expected.hpp"

namespace exage::Renderer
{
    struct AssetImportResult
    {
        std::vector<std::unique_ptr<Texture>> textures;
        std::vector<std::unique_ptr<Material>> materials;
        std::vector<std::unique_ptr<Mesh>> meshes;

        struct Node
        {
            Transform3D transform;
            size_t meshIndex = 0;
            Node* parent = nullptr;
            std::vector<std::unique_ptr<Node>> children;
        };

        std::vector<std::unique_ptr<Node>> rootNodes;
    };

    [[nodiscard]] EXAGE_EXPORT auto importAsset(const std::filesystem::path& assetPath) noexcept
        -> tl::expected<AssetImportResult, AssetImportError>;

    [[nodiscard]] EXAGE_EXPORT auto importTexture(const std::filesystem::path& texturePath) noexcept
        -> tl::expected<Texture, TextureImportError>;

    [[nodiscard]] EXAGE_EXPORT auto saveAssets(const AssetImportResult& assets,
                                               const std::filesystem::path& saveDirectory,
                                               const std::filesystem::path& prefix) noexcept
        -> std::optional<DirectoryError>;

    [[nodiscard]] EXAGE_EXPORT auto saveTexture(Texture& texture,
                                                const std::filesystem::path& savePath,
                                                const std::filesystem::path& prefix) noexcept
        -> std::optional<SaveError>;

    struct AssetSceneImportInfo
    {
        std::span<GPUMesh> meshes;

        std::span<std::unique_ptr<AssetImportResult::Node>> rootNodes;
    };

    EXAGE_EXPORT void importScene(const AssetSceneImportInfo& info,
                                  Scene& scene,
                                  Entity parent = entt::null) noexcept;

}  // namespace exage::Renderer
