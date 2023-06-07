#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include <vcruntime.h>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Loader/AssetFile.h"
#include "exage/Renderer/Scene/Loader/Errors.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/Scene/Scene.h"
#include "tl/expected.hpp"

namespace exage::Renderer
{
    struct AssetImportResult2
    {
        std::vector<std::filesystem::path> textures;

        struct Material
        {
            size_t albedoTextureIndex = std::numeric_limits<size_t>::max();
            glm::vec3 albedoColor = glm::vec3(1.0f);
            size_t normalTextureIndex = std::numeric_limits<size_t>::max();
            size_t metallicTextureIndex = std::numeric_limits<size_t>::max();
            float metallicValue = 0.0f;
            size_t roughnessTextureIndex = std::numeric_limits<size_t>::max();
            float roughnessValue = 0.0f;
            size_t aoTextureIndex = std::numeric_limits<size_t>::max();
            float aoValue = 0.0f;
            size_t emissiveTextureIndex = std::numeric_limits<size_t>::max();
            glm::vec3 emissiveColor = glm::vec3(0.0f);
        };

        std::vector<Material> materials;

        struct Mesh
        {
            std::vector<MeshVertex> vertices;
            std::vector<uint32_t> indices;
            AABB aabb;
            size_t materialIndex = 0;
        };

        std::vector<Mesh> meshes;

        struct Node
        {
            Transform3D transform;
            size_t meshIndex = 0;
            size_t parentIndex = std::numeric_limits<size_t>::max();
            std::vector<size_t> childrenIndices;
        };

        std::vector<Node> nodes;
        std::vector<size_t> rootNodes;
    };

    [[nodiscard]] auto importAsset2(const std::filesystem::path& assetPath) noexcept
        -> tl::expected<AssetImportResult2, AssetError>;

    [[nodiscard]] auto importTexture(const std::filesystem::path& texturePath) noexcept
        -> tl::expected<Texture, AssetError>;

    [[nodiscard]] auto saveTexture(Texture& texture) noexcept -> AssetFile;
    [[nodiscard]] auto saveMaterial(Material& material) noexcept -> AssetFile;
    [[nodiscard]] auto saveMesh(Mesh& mesh) noexcept -> AssetFile;

    [[nodiscard]] auto saveTexture(Texture& texture,
                                   const std::filesystem::path& savePath,
                                   const std::filesystem::path& prefix) noexcept
        -> tl::expected<void, AssetError>;
    [[nodiscard]] auto saveMaterial(Material& material,
                                    const std::filesystem::path& savePath,
                                    const std::filesystem::path& prefix) noexcept
        -> tl::expected<void, AssetError>;
    [[nodiscard]] auto saveMesh(Mesh& mesh,
                                const std::filesystem::path& savePath,
                                const std::filesystem::path& prefix) noexcept
        -> tl::expected<void, AssetError>;

    struct AssetSceneImportInfo
    {
        std::span<GPUMesh> meshes;

        std::span<const size_t> rootNodes;
        std::span<const AssetImportResult2::Node> nodes;
    };

    void importScene(const AssetSceneImportInfo& info,
                     Scene& scene,
                     Entity parent = entt::null) noexcept;

}  // namespace exage::Renderer
