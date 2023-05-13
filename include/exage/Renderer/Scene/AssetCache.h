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

        EXAGE_DEFAULT_COPY(AssetCache);
        EXAGE_DEFAULT_MOVE(AssetCache);

        void addTexture(GPUTexture texture) noexcept { _textures.emplace(texture.path, texture); }
        void addMesh(GPUMesh mesh) noexcept { _meshes.emplace(mesh.path, mesh); }
        void addMaterial(GPUMaterial material) noexcept
        {
            _materials.emplace(material.path, material);
        }

        [[nodiscard]] auto getTexture(const std::filesystem::path& path) noexcept -> GPUTexture
        {
            return _textures[path];
        }
        [[nodiscard]] auto getMesh(const std::filesystem::path& path) noexcept -> GPUMesh
        {
            return _meshes[path];
        }
        [[nodiscard]] auto getMaterial(const std::filesystem::path& path) noexcept -> GPUMaterial
        {
            return _materials[path];
        }

        [[nodiscard]] auto hasTexture(const std::filesystem::path& path) const noexcept -> bool
        {
            return _textures.contains(path);
        }
        [[nodiscard]] auto hasMesh(const std::filesystem::path& path) const noexcept -> bool
        {
            return _meshes.contains(path);
        }
        [[nodiscard]] auto hasMaterial(const std::filesystem::path& path) const noexcept -> bool
        {
            return _materials.contains(path);
        }

      private:
        std::unordered_map<std::filesystem::path, GPUTexture, Filesystem::PathHash> _textures;
        std::unordered_map<std::filesystem::path, GPUMesh, Filesystem::PathHash> _meshes;
        std::unordered_map<std::filesystem::path, GPUMaterial, Filesystem::PathHash> _materials;
    };
}  // namespace exage::Renderer
