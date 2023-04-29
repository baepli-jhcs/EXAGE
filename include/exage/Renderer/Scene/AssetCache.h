#pragma once

#include <filesystem>

#include "exage/Core/Core.h"
#include "exage/Filesystem/Directories.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{
    class EXAGE_EXPORT AssetCache
    {
      public:
        AssetCache() noexcept = default;
        ~AssetCache() = default;

        EXAGE_DELETE_COPY(AssetCache);
        EXAGE_DEFAULT_MOVE(AssetCache);

        void addTexture(std::unique_ptr<Texture> texture) noexcept;
        void addMesh(std::unique_ptr<Mesh> mesh) noexcept;
        void addMaterial(std::unique_ptr<Material> material) noexcept;

        [[nodiscard]] auto getTexture(const std::filesystem::path& path) noexcept -> Texture*;
        [[nodiscard]] auto getMesh(const std::filesystem::path& path) noexcept -> Mesh*;
        [[nodiscard]] auto getMaterial(const std::filesystem::path& path) noexcept -> Material*;

      private:
        std::unordered_map<std::filesystem::path, std::unique_ptr<Texture>, Filesystem::PathHash>
            _textures;
        std::unordered_map<std::filesystem::path, std::unique_ptr<Mesh>, Filesystem::PathHash>
            _meshes;
        std::unordered_map<std::filesystem::path, std::unique_ptr<Material>, Filesystem::PathHash>
            _materials;
    };
}  // namespace exage::Renderer
