#include <fstream>

#include "exage/Renderer/Scene/Loader/Loader.h"

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>

namespace exage::Renderer
{
    EXAGE_EXPORT auto loadTexture(const std::filesystem::path& path) noexcept
        -> tl::expected<Texture, AssetLoadError>
    {
        if (!std::filesystem::exists(path))
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        if (path.extension() != TEXTURE_EXTENSION)
        {
            return tl::make_unexpected(FileFormatError {});
        }

        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        Texture texture;
        cereal::BinaryInputArchive archive(file);
        archive(texture);

        return texture;
    }

    auto loadMaterial(const std::filesystem::path& path) noexcept
        -> tl::expected<Material, AssetLoadError>
    {
        if (!std::filesystem::exists(path))
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        if (path.extension() != MATERIAL_EXTENSION)
        {
            return tl::make_unexpected(FileFormatError {});
        }

        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        Material material;
        cereal::BinaryInputArchive archive(file);
        archive(material);

        return material;
    }

    auto loadMesh(const std::filesystem::path& path) noexcept -> tl::expected<Mesh, AssetLoadError>
    {
        if (!std::filesystem::exists(path))
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        if (path.extension() != MESH_EXTENSION)
        {
            return tl::make_unexpected(FileFormatError {});
        }

        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        Mesh mesh;
        cereal::BinaryInputArchive archive(file);
        archive(mesh);

        return mesh;
    }

    auto uploadTexture(const Texture& texture, const TextureUploadOptions& options) noexcept
        -> GPUTexture
    {
        GPUTexture gpuTexture;

        std::shared_ptr<Graphics::Buffer> stagingBuffer;
    }

    auto uploadMesh(const Mesh& mesh, const MeshUploadOptions& options) noexcept -> GPUMesh
    {
        GPUMesh gpuMesh;
        gpuMesh.path = mesh.path;
        gpuMesh.materialPath = mesh.materialPath;
        gpuMesh.aabb = mesh.aabb;

        gpuMesh.lods.resize(mesh.lods.size());

        for (size_t i = 0; i < mesh.lods.size(); i++)
        {
            const auto& lod = mesh.lods[i];
            auto& gpuLod = gpuMesh.lods[i];

            std::span<const std::byte> vertexData = std::as_bytes(std::span(lod.vertices));

            gpuLod.vertexOffset = options.sceneBuffer.uploadData(
                options.commandBuffer, vertexData, options.access, options.pipelineStage);
            gpuLod.vertexCount = static_cast<uint32_t>(lod.vertices.size());

            std::span<const std::byte> indexData = std::as_bytes(std::span(lod.indices));

            gpuLod.indexOffset = options.sceneBuffer.uploadData(
                options.commandBuffer, indexData, options.access, options.pipelineStage);
            gpuLod.indexCount = static_cast<uint32_t>(lod.indices.size());
        }

        return gpuMesh;
    }

}  // namespace exage::Renderer
