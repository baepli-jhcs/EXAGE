#include <fstream>

#include "exage/Renderer/Scene/Loader/Loader.h"

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <vcruntime.h>

#include "exage/Core/Debug.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Material.h"

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
        debugAssume(!texture.mips.empty(), "Texture must have at least one mip level");

        GPUTexture gpuTexture;
        gpuTexture.path = texture.path;

        std::shared_ptr<Graphics::Buffer> stagingBuffer;

        Graphics::BufferCreateInfo stagingBufferCreateInfo;
        stagingBufferCreateInfo.size = texture.data.size();
        stagingBufferCreateInfo.cached = false;
        stagingBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

        stagingBuffer = options.context.createBuffer(stagingBufferCreateInfo);
        std::span<const std::byte> data = std::as_bytes(std::span(texture.data));

        stagingBuffer->write(data, 0);

        Graphics::TextureCreateInfo textureCreateInfo;
        textureCreateInfo.extent = texture.mips[0].extent;
        textureCreateInfo.format = texture.format;
        textureCreateInfo.usage = options.usage;  // Graphics::Texture::UsageFlags::eSampled |
        // Graphics::Texture::UsageFlags::eTransferDst;

        debugAssume(textureCreateInfo.usage.any(Graphics::Texture::UsageFlags::eTransferDst),
                    "Texture must be transferable");

        textureCreateInfo.mipLevels = static_cast<uint32_t>(texture.mips.size());

        textureCreateInfo.type = Graphics::Texture::Type::e2D;
        textureCreateInfo.arrayLayers = 1;

        textureCreateInfo.samplerCreateInfo = options.samplerCreateInfo;

        gpuTexture.texture = options.context.createTexture(textureCreateInfo);

        options.commandBuffer.textureBarrier(gpuTexture.texture,
                                             Graphics::Texture::Layout::eTransferDst,
                                             Graphics::PipelineStageFlags::eTopOfPipe,
                                             Graphics::PipelineStageFlags::eTransfer,
                                             Graphics::Access {},
                                             Graphics::AccessFlags::eTransferWrite);

        for (size_t i = 0; i < texture.mips.size(); i++)
        {
            const auto& mip = texture.mips[i];

            size_t offset = mip.offset;
            size_t size = mip.size;

            options.commandBuffer.copyBufferToTexture(
                stagingBuffer, gpuTexture.texture, offset, glm::vec3 {0}, i, 0, 1, mip.extent);
        }

        options.commandBuffer.textureBarrier(gpuTexture.texture,
                                             options.layout,
                                             Graphics::PipelineStageFlags::eTransfer,
                                             options.pipelineStage,
                                             Graphics::AccessFlags::eTransferRead,
                                             options.access);

        return gpuTexture;
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
