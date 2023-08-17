#include <fstream>
#include <unordered_set>

#include "exage/Renderer/Scene/Loader/Loader.h"

#include <exage/utils/serialization.h>

#include "exage/Core/Debug.h"
#include "exage/Core/Errors.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Loader/AssetFile.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "zstd.h"

namespace exage::Renderer
{
    namespace
    {
        [[nodiscard]] auto getUncompressedFormat(uint8_t channels, uint8_t bitsPerChannel) noexcept
            -> Graphics::Format
        {
            switch (bitsPerChannel)
            {
                case 8:
                {
                    switch (channels)
                    {
                        case 1:
                        {
                            return Graphics::Format::eR8;
                        }

                        case 2:
                        {
                            return Graphics::Format::eRG8;
                        }

                        case 4:
                        {
                            return Graphics::Format::eRGBA8;
                        }
                    }
                }

                case 16:
                {
                    switch (channels)
                    {
                        case 1:
                        {
                            return Graphics::Format::eR16f;
                        }

                        case 2:
                        {
                            return Graphics::Format::eRG16f;
                        }

                        case 4:
                        {
                            return Graphics::Format::eRGBA16f;
                        }
                    }
                }

                case 32:
                {
                    switch (channels)
                    {
                        case 1:
                        {
                            return Graphics::Format::eR32f;
                        }

                        case 2:
                        {
                            return Graphics::Format::eRG32f;
                        }

                        case 4:
                        {
                            return Graphics::Format::eRGBA32f;
                        }
                    }
                }

                default:
                    break;
            }

            debugAssume(false, "Invalid format");  // Should have been handled by the converter
            return Graphics::Format::eRGBA8;
        }

        [[nodiscard]] auto getCompressedFormat(
            uint8_t channels,
            uint8_t bitsPerChannel,
            const std::unordered_set<Graphics::Format>& compressedTextureSupport)
            -> Graphics::Format
        {
            // Priority order: ASTC > BC > ETC2

            if (bitsPerChannel != 8)
            {
                return getUncompressedFormat(channels, bitsPerChannel);
            }

            switch (channels)
            {
                case 1:
                {
                    if (compressedTextureSupport.contains(Graphics::Format::eBC4R8))
                    {
                        return Graphics::Format::eBC4R8;
                    }
                }

                case 2:
                {
                    if (compressedTextureSupport.contains(Graphics::Format::eBC5RG8))
                    {
                        return Graphics::Format::eBC5RG8;
                    }
                }

                case 4:
                {
                    if (compressedTextureSupport.contains(Graphics::Format::eASTC6x6RGBA8))
                    {
                        return Graphics::Format::eASTC6x6RGBA8;
                    }

                    if (compressedTextureSupport.contains(Graphics::Format::eASTC4x4RGBA8))
                    {
                        return Graphics::Format::eASTC4x4RGBA8;
                    }

                    if (compressedTextureSupport.contains(Graphics::Format::eBC7RGBA8))
                    {
                        return Graphics::Format::eBC7RGBA8;
                    }

                    if (compressedTextureSupport.contains(Graphics::Format::eBC3RGBA8))
                    {
                        return Graphics::Format::eBC3RGBA8;
                    }

                    if (compressedTextureSupport.contains(Graphics::Format::eBC1RGBA8))
                    {
                        return Graphics::Format::eBC1RGBA8;
                    }

                    if (compressedTextureSupport.contains(Graphics::Format::eETC2RGBA8))
                    {
                        return Graphics::Format::eETC2RGBA8;
                    }
                }
            }

            return getUncompressedFormat(channels, bitsPerChannel);
        }
    }  // namespace

    auto queryCompressedTextureSupport(Graphics::Context& context) noexcept
        -> std::unordered_set<Graphics::Format>
    {
        constexpr std::array compressedFormats = {
            Graphics::Format::eBC1RGBA8,
            Graphics::Format::eBC3RGBA8,
            Graphics::Format::eBC4R8,
            Graphics::Format::eBC5RG8,
            Graphics::Format::eBC7RGBA8,
            Graphics::Format::eASTC4x4RGBA8,
            Graphics::Format::eASTC6x6RGBA8,
            Graphics::Format::eETC2RGBA8,
        };

        std::unordered_set<Graphics::Format> formats;

        for (Graphics::Format format : compressedFormats)
        {
            if (context.getFormatSupport(format).first)
            {
                formats.insert(format);
            }
        }

        return formats;
    }

    auto loadTexture(const std::filesystem::path& path) noexcept -> tl::expected<Texture, Error>
    {
        tl::expected asset = loadAssetFile(path);

        if (!asset.has_value())
        {
            return tl::make_unexpected(asset.error());
        }

        nlohmann::json json = nlohmann::json::parse(asset->json);

        if (json["dataType"] != "Texture")
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

        Texture texture;
        texture.path = json["path"].get<std::string>();
        texture.mips.resize(json["mips"].size());

        for (size_t i = 0; i < json["mips"].size(); i++)
        {
            const auto& mip = json["mips"][i];
            texture.mips[i].extent = mip["extent"];
            texture.mips[i].offset = mip["offset"];
            texture.mips[i].size = mip["size"];
        }

        texture.channels = json["channels"];
        texture.bitsPerChannel = json["bitsPerChannel"];
        texture.type = static_cast<Graphics::Texture::Type>(json["type"]);

        size_t decompressedSize = json["rawSize"];
        texture.data.resize(decompressedSize);

        size_t result = ZSTD_decompress(
            texture.data.data(), texture.data.size(), asset->binary.data(), asset->binary.size());

        if (ZSTD_isError(result) != 0u)
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

        return texture;
    }

    auto loadMaterial(const std::filesystem::path& path) noexcept -> tl::expected<Material, Error>
    {
        tl::expected asset = loadAssetFile(path);

        if (!asset.has_value())
        {
            return tl::make_unexpected(asset.error());
        }

        nlohmann::json json = nlohmann::json::parse(asset->json);

        if (json["dataType"] != "Material")
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

        Material material;
        material.path = json["path"].get<std::string>();
        material.albedoColor = json["albedoColor"];
        material.emissiveColor = json["emissiveColor"];
        material.metallicValue = json["metallicValue"];
        material.roughnessValue = json["roughnessValue"];
        material.albedoUseTexture = json["albedoUseTexture"];
        material.normalUseTexture = json["normalUseTexture"];
        material.metallicUseTexture = json["metallicUseTexture"];
        material.roughnessUseTexture = json["roughnessUseTexture"];
        material.occlusionUseTexture = json["occlusionUseTexture"];
        material.emissiveUseTexture = json["emissiveUseTexture"];
        material.albedoTexturePath = json["albedoTexturePath"].get<std::string>();
        material.normalTexturePath = json["normalTexturePath"].get<std::string>();
        material.metallicTexturePath = json["metallicTexturePath"].get<std::string>();
        material.roughnessTexturePath = json["roughnessTexturePath"].get<std::string>();
        material.occlusionTexturePath = json["occlusionTexturePath"].get<std::string>();
        material.emissiveTexturePath = json["emissiveTexturePath"].get<std::string>();

        return material;
    }

    auto loadMesh(const std::filesystem::path& path) noexcept -> tl::expected<StaticMesh, Error>
    {
        tl::expected asset = loadAssetFile(path);

        if (!asset.has_value())
        {
            return tl::make_unexpected(asset.error());
        }

        nlohmann::json json = nlohmann::json::parse(asset->json);

        if (json["dataType"] != "StaticMesh")
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

        StaticMesh mesh;
        mesh.path = json["path"].get<std::string>();
        mesh.materialPath = json["materialPath"].get<std::string>();
        mesh.aabb.max = json["aabb"]["max"];
        mesh.aabb.min = json["aabb"]["min"];
        mesh.lods.resize(json["lods"].size());

        for (size_t i = 0; i < mesh.lods.size(); i++)
        {
            const auto& lod = json["lods"][i];
            auto& meshLod = mesh.lods[i];
            meshLod.vertexCount = lod["vertexCount"];
            meshLod.indexCount = lod["indexCount"];
            meshLod.vertexOffset = lod["vertexOffset"];
            meshLod.indexOffset = lod["indexOffset"];
        }

        size_t vertices = json["vertices"];
        size_t indices = json["indices"];

        size_t vertexCompressedSize = json["vertexCompressedSize"];
        size_t indexCompressedSize = json["indexCompressedSize"];

        mesh.vertices.resize(vertices);
        mesh.indices.resize(indices);

        size_t result = ZSTD_decompress(mesh.vertices.data(),
                                        mesh.vertices.size() * sizeof(StaticMeshVertex),
                                        asset->binary.data(),
                                        vertexCompressedSize);

        if (ZSTD_isError(result) != 0u)
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

        result = ZSTD_decompress(mesh.indices.data(),
                                 mesh.indices.size() * sizeof(uint32_t),
                                 asset->binary.data() + vertexCompressedSize,
                                 indexCompressedSize);

        if (ZSTD_isError(result) != 0u)
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

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

        std::unordered_set<Graphics::Format>* supportedCompressedFormatsPtr =
            options.supportedCompressedFormats;

        std::optional<std::unordered_set<Graphics::Format>> supportedCompressedFormats =
            std::nullopt;
        if (options.useCompressedFormat)
        {
            if (supportedCompressedFormatsPtr == nullptr)
            {
                supportedCompressedFormats = queryCompressedTextureSupport(options.context);
                supportedCompressedFormatsPtr = &*supportedCompressedFormats;
            }
        }

        Graphics::TextureCreateInfo textureCreateInfo;
        textureCreateInfo.extent = texture.mips[0].extent;
        textureCreateInfo.format = options.useCompressedFormat
            ? getCompressedFormat(
                texture.channels, texture.bitsPerChannel, *supportedCompressedFormatsPtr)
            : getUncompressedFormat(texture.channels, texture.bitsPerChannel);
        textureCreateInfo.usage = options.usage;  // Graphics::Texture::UsageFlags::eSampled |
        // Graphics::Texture::UsageFlags::eTransferDst;

        debugAssume(textureCreateInfo.usage.any(Graphics::Texture::UsageFlags::eTransferDst),
                    "Texture must be transferable");

        textureCreateInfo.mipLevels = static_cast<uint32_t>(texture.mips.size());

        textureCreateInfo.type = Graphics::Texture::Type::e2D;
        textureCreateInfo.arrayLayers = 1;

        gpuTexture.texture = options.context.createTexture(textureCreateInfo);

        options.commandBuffer.textureBarrier(gpuTexture.texture,
                                             Graphics::Texture::Layout::eTransferDst,
                                             Graphics::PipelineStageFlags::eTopOfPipe,
                                             Graphics::PipelineStageFlags::eTransfer,
                                             Graphics::Access {},
                                             Graphics::AccessFlags::eTransferWrite);

        for (uint32_t i = 0; i < texture.mips.size(); i++)
        {
            const auto& mip = texture.mips[i];

            size_t offset = mip.offset;

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

    auto uploadMesh(const StaticMesh& mesh, const MeshUploadOptions& options) noexcept
        -> GPUStaticMesh
    {
        GPUStaticMesh gpuMesh;
        gpuMesh.path = mesh.path;

        std::hash<std::string> hasher;
        gpuMesh.pathHash = hasher(mesh.path);
        gpuMesh.materialPath = mesh.materialPath;
        gpuMesh.aabb = mesh.aabb;

        gpuMesh.lods = mesh.lods;

        size_t vertexBufferSize = mesh.vertices.size() * sizeof(StaticMeshVertex);
        size_t indexBufferSize = mesh.indices.size() * sizeof(uint32_t);

        Graphics::BufferCreateInfo vertexBufferCreateInfo;
        vertexBufferCreateInfo.size = vertexBufferSize;
        vertexBufferCreateInfo.cached = false;
        vertexBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eIfOptimal;

        Graphics::BufferCreateInfo indexBufferCreateInfo;
        indexBufferCreateInfo.size = indexBufferSize;
        indexBufferCreateInfo.cached = false;
        indexBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eIfOptimal;

        gpuMesh.vertexBuffer = options.context.createBuffer(vertexBufferCreateInfo);
        gpuMesh.indexBuffer = options.context.createBuffer(indexBufferCreateInfo);

        std::span<const std::byte> vertexData = std::as_bytes(std::span(mesh.vertices));
        std::span<const std::byte> indexData = std::as_bytes(std::span(mesh.indices));

        if (gpuMesh.vertexBuffer->isMapped())
        {
            gpuMesh.vertexBuffer->write(vertexData, 0);
        }
        else
        {
            Graphics::BufferCreateInfo stagingBufferCreateInfo;
            stagingBufferCreateInfo.size = vertexData.size();
            stagingBufferCreateInfo.cached = false;
            stagingBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

            auto stagingBuffer = options.context.createBuffer(stagingBufferCreateInfo);
            stagingBuffer->write(vertexData, 0);

            options.commandBuffer.copyBuffer(
                stagingBuffer, gpuMesh.vertexBuffer, 0, 0, vertexData.size());
            options.commandBuffer.bufferBarrier(gpuMesh.vertexBuffer,
                                                Graphics::PipelineStageFlags::eTransfer,
                                                options.pipelineStage,
                                                Graphics::AccessFlags::eTransferWrite,
                                                options.access);
        }

        if (gpuMesh.indexBuffer->isMapped())
        {
            gpuMesh.indexBuffer->write(indexData, 0);
        }
        else
        {
            Graphics::BufferCreateInfo stagingBufferCreateInfo;
            stagingBufferCreateInfo.size = indexData.size();
            stagingBufferCreateInfo.cached = false;
            stagingBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

            auto stagingBuffer = options.context.createBuffer(stagingBufferCreateInfo);
            stagingBuffer->write(indexData, 0);

            options.commandBuffer.copyBuffer(
                stagingBuffer, gpuMesh.indexBuffer, 0, 0, indexData.size());
            options.commandBuffer.bufferBarrier(gpuMesh.indexBuffer,
                                                Graphics::PipelineStageFlags::eTransfer,
                                                options.pipelineStage,
                                                Graphics::AccessFlags::eTransferWrite,
                                                options.access);
        }

        // for (size_t i = 0; i < mesh.lods.size(); i++)
        // {
        //     const auto& lod = mesh.lods[i];
        //     auto& gpuLod = gpuMesh.lods[i];

        //     std::span<const std::byte> vertexData =
        //         std::as_bytes(std::span(mesh.vertices))
        //             .subspan(lod.vertexOffset, lod.vertexCount * sizeof(StaticMeshVertex));

        //     std::span<const std::byte> indexData =
        //         std::as_bytes(std::span(mesh.indices))
        //             .subspan(lod.indexOffset, lod.indexCount * sizeof(uint32_t));

        //     {
        //         Graphics::BufferCreateInfo stagingBufferCreateInfo;
        //         stagingBufferCreateInfo.size = vertexData.size();
        //         stagingBufferCreateInfo.cached = false;
        //         stagingBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

        //         auto stagingBuffer = options.context.createBuffer(stagingBufferCreateInfo);
        //         stagingBuffer->write(vertexData, 0);

        //         options.commandBuffer.copyBuffer(
        //             stagingBuffer, gpuMesh.vertexBuffer, 0, 0, vertexData.size());
        //         options.commandBuffer.bufferBarrier(gpuMesh.vertexBuffer,
        //                                             Graphics::PipelineStageFlags::eTransfer,
        //                                             options.pipelineStage,
        //                                             Graphics::AccessFlags::eTransferWrite,
        //                                             options.access);
        //     }

        //     {
        //         Graphics::BufferCreateInfo stagingBufferCreateInfo;
        //         stagingBufferCreateInfo.size = indexData.size();
        //         stagingBufferCreateInfo.cached = false;
        //         stagingBufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

        //         auto stagingBuffer = options.context.createBuffer(stagingBufferCreateInfo);
        //         stagingBuffer->write(indexData, 0);

        //         options.commandBuffer.copyBuffer(
        //             stagingBuffer, gpuMesh.indexBuffer, 0, 0, indexData.size());
        //         options.commandBuffer.bufferBarrier(gpuMesh.indexBuffer,
        //                                             Graphics::PipelineStageFlags::eTransfer,
        //                                             options.pipelineStage,
        //                                             Graphics::AccessFlags::eTransferWrite,
        //                                             options.access);
        //     }

        //     gpuLod.vertexCount = lod.vertexCount;
        //     gpuLod.indexCount = lod.indexCount;

        //     gpuLod.vertexOffset = lod.vertexOffset;
        //     gpuLod.indexOffset = lod.indexOffset;

        //     // gpuLod.vertexOffset = options.sceneBuffer.uploadData(
        //     //     vertices, options.commandBuffer, options.access, options.pipelineStage);
        //     // gpuLod.vertexCount = static_cast<uint32_t>(lod.vertices.size());

        //     // std::span<const uint32_t> indexData = lod.indices;

        //     // gpuLod.indexOffset = options.sceneBuffer.uploadData(
        //     //     indexData, options.commandBuffer, options.access, options.pipelineStage);
        //     // gpuLod.indexCount = static_cast<uint32_t>(lod.indices.size());
        // }

        return gpuMesh;
    }

}  // namespace exage::Renderer
