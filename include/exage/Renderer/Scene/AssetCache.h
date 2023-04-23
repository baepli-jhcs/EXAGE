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

        void addTexture(std::weak_ptr<Texture> texture) noexcept;
        void addMesh(Mesh mesh) noexcept;
        void addMaterial(Material material) noexcept;

        [[nodiscard]] auto getTexture(const std::filesystem::path& path) noexcept
            -> std::shared_ptr<Texture>;
        [[nodiscard]] auto getMesh(const std::filesystem::path& path) noexcept
            -> Mesh;
        [[nodiscard]] auto getMaterial(const std::filesystem::path& path) noexcept
            -> Material;

      private:
        std::unordered_map<std::filesystem::path, std::weak_ptr<Texture>> _textures;
        std::unordered_map<std::filesystem::path, Mesh> _meshes;
        std::unordered_map<std::filesystem::path, Material> _materials;
    };
}  // namespace exage::Renderer
