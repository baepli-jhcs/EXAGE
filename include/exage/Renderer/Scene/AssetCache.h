#pragma once

#include <filesystem>

#include "exage/Core/Core.h"
#include "exage/Filesystem/Directories.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{
    class AssetCache
    {
      public:
        AssetCache() noexcept = default;
        ~AssetCache() = default;

        EXAGE_DEFAULT_COPY(AssetCache);
        EXAGE_DEFAULT_MOVE(AssetCache);

        void addTexture(GPUTexture texture) noexcept { _textures.emplace(texture.path, texture); }
        void addMesh(GPUMesh mesh) noexcept { _meshes.emplace(mesh.pathHash, mesh); }
        void addMaterial(GPUMaterial material) noexcept
        {
            _materials.emplace(material.path, material);
        }

        [[nodiscard]] auto getTexture(const std::filesystem::path& path) noexcept -> GPUTexture&
        {
            return _textures[path];
        }
        [[nodiscard]] auto getMesh(const std::filesystem::path& path) noexcept -> GPUMesh&
        {
            size_t hash = std::filesystem::hash_value(path);
            return _meshes[hash];
        }
        [[nodiscard]] auto getMaterial(const std::filesystem::path& path) noexcept -> GPUMaterial&
        {
            return _materials[path];
        }

        [[nodiscard]] auto hasTexture(const std::filesystem::path& path) const noexcept -> bool
        {
            return _textures.contains(path);
        }
        [[nodiscard]] auto hasMesh(const std::filesystem::path& path) const noexcept -> bool
        {
            size_t hash = std::filesystem::hash_value(path);
            return _meshes.contains(hash);
        }
        [[nodiscard]] auto hasMaterial(const std::filesystem::path& path) const noexcept -> bool
        {
            return _materials.contains(path);
        }

        [[nodiscard]] auto getMesh(size_t hash) noexcept -> GPUMesh& { return _meshes[hash]; }
        [[nodiscard]] auto hasMesh(size_t hash) const noexcept -> bool
        {
            return _meshes.contains(hash);
        }
        [[nodiscard]] auto getMeshIfExists(size_t hash) noexcept -> GPUMesh*
        {
            auto it = _meshes.find(hash);
            if (it != _meshes.end())
            {
                return &it->second;
            }

            return nullptr;
        }

      private:
        std::unordered_map<std::filesystem::path, GPUTexture, Filesystem::PathHash> _textures;
        std::unordered_map<size_t, GPUMesh> _meshes;
        std::unordered_map<std::filesystem::path, GPUMaterial, Filesystem::PathHash> _materials;
    };
}  // namespace exage::Renderer
