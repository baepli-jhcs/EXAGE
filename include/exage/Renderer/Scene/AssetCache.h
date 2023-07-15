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
        void addMesh(GPUStaticMesh mesh) noexcept { _meshes.emplace(mesh.pathHash, mesh); }
        void addMaterial(GPUMaterial material) noexcept
        {
            _materials.emplace(material.path, material);
        }

        [[nodiscard]] auto getTexture(const std::string& path) noexcept -> GPUTexture&
        {
            return _textures[path];
        }
        [[nodiscard]] auto getMesh(const std::string& path) noexcept -> GPUStaticMesh&
        {
            size_t hash = std::filesystem::hash_value(path);
            return _meshes[hash];
        }
        [[nodiscard]] auto getMaterial(const std::string& path) noexcept -> GPUMaterial&
        {
            return _materials[path];
        }

        [[nodiscard]] auto hasTexture(const std::string& path) const noexcept -> bool
        {
            return _textures.contains(path);
        }
        [[nodiscard]] auto hasMesh(const std::string& path) const noexcept -> bool
        {
            size_t hash = std::filesystem::hash_value(path);
            return _meshes.contains(hash);
        }
        [[nodiscard]] auto hasMaterial(const std::string& path) const noexcept -> bool
        {
            return _materials.contains(path);
        }

        [[nodiscard]] auto getMesh(size_t hash) noexcept -> GPUStaticMesh& { return _meshes[hash]; }
        [[nodiscard]] auto hasMesh(size_t hash) const noexcept -> bool
        {
            return _meshes.contains(hash);
        }
        [[nodiscard]] auto getMeshIfExists(size_t hash) noexcept -> GPUStaticMesh*
        {
            auto it = _meshes.find(hash);
            if (it != _meshes.end())
            {
                return &it->second;
            }

            return nullptr;
        }

        [[nodiscard]] auto getMeshIfExists(size_t hash) const noexcept -> const GPUStaticMesh*
        {
            auto it = _meshes.find(hash);
            if (it != _meshes.end())
            {
                return &it->second;
            }

            return nullptr;
        }

      private:
        std::unordered_map<std::string, GPUTexture> _textures;
        std::unordered_map<size_t, GPUStaticMesh> _meshes;
        std::unordered_map<std::string, GPUMaterial> _materials;
    };
}  // namespace exage::Renderer
