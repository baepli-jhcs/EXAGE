#pragma once

#include <filesystem>

#include "exage/Core/Core.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"

namespace exage::Renderer
{
    class EXAGE_EXPORT AssetCache
    {
      public:
        AssetCache() noexcept = default;
        ~AssetCache() = default;

        void addTexture(std::unique_ptr<Texture> texture) noexcept;
        void addMesh(std::unique_ptr<Mesh> mesh) noexcept;
        void addMaterial(std::unique_ptr<Material> material) noexcept;

        [[nodiscard]] auto getTexture(const std::filesystem::path& path) noexcept -> Texture*;
        [[nodiscard]] auto getMesh(const std::filesystem::path& path) noexcept -> Mesh*;
        [[nodiscard]] auto getMaterial(const std::filesystem::path& path) noexcept -> Material*;

      private:
        std::unordered_map<std::filesystem::path, std::unique_ptr<Texture>> _textures;
        std::unordered_map<std::filesystem::path, std::unique_ptr<Mesh>> _meshes;
        std::unordered_map<std::filesystem::path, std::unique_ptr<Material>> _materials;
    };
}  // namespace exage::Renderer
