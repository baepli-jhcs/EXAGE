#include <fstream>

#include "exage/Renderer/Scene/Loader/Loader.h"

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>

#include "exage/Core/Debug.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Texture.h"

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

        Graphics::BufferCreateInfo stagingBufferCreateInfo;
        stagingBufferCreateInfo.size = texture.data.size();
        stagingBufferCreateInfo.cached = false;
        stagingBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

        stagingBuffer = options.context.createBuffer(stagingBufferCreateInfo);
        std::span<const std::byte> data = std::as_bytes(std::span(texture.data));

        stagingBuffer->write(data, 0);

        Graphics::TextureCreateInfo textureCreateInfo;
        textureCreateInfo.extent = texture.extent;
        textureCreateInfo.format = texture.format;
        textureCreateInfo.usage = options.usage;  // Graphics::Texture::UsageFlags::eSampled |
        // Graphics::Texture::UsageFlags::eTransferDst;

        debugAssume(textureCreateInfo.usage.any(Graphics::Texture::UsageFlags::eTransferDst),
                    "Texture must be transferable");

        if (options.generateMipmaps)
        {
            auto mipLevels = static_cast<uint32_t>(std::floor(
                                 std::log2(std::max(texture.extent.x, texture.extent.y))))
                + 1;
            textureCreateInfo.mipLevels = mipLevels;

            textureCreateInfo.usage |= Graphics::Texture::UsageFlags::eTransferSrc;
        }
        else
        {
            textureCreateInfo.mipLevels = 1;
        }

        textureCreateInfo.type = Graphics::Texture::Type::e2D;
        textureCreateInfo.arrayLayers = 1;

        textureCreateInfo.samplerCreateInfo = options.samplerCreateInfo;

        gpuTexture.texture = options.context.createTexture(textureCreateInfo);

        options.commandBuffer.copyBufferToTexture(
            stagingBuffer, gpuTexture.texture, 0, glm::vec3 {0}, 0, 0, 1, texture.extent);

        if (options.generateMipmaps)
        {
            debugAssume(textureCreateInfo.usage.any(Graphics::Texture::UsageFlags::eTransferSrc),
                        "If mipmaps are generated, the texture must be src transferable");

            for (uint32_t i = 0; i < textureCreateInfo.mipLevels; i++)
            {
            }
        }
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
