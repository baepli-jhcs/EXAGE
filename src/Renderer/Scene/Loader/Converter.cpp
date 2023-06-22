#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "exage/Renderer/Scene/Loader/Converter.h"

#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <ktx.h>
#include <stb_image.h>
#include <tl/expected.hpp>
#include <zstd.h>

#include "exage/Filesystem/Directories.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Loader/AssetFile.h"
#include "exage/Renderer/Scene/Loader/Errors.h"
#include "exage/Renderer/Scene/Loader/Loader.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/platform/Vulkan/VulkanUtils.h"
#include "ktxvulkan.h"

namespace exage::Renderer
{
    namespace
    {
        [[nodiscard]] auto processMaterial2(
            const std::filesystem::path& assetDirectory,
            const aiMaterial& material,
            std::vector<std::filesystem::path>& textures,
            std::unordered_map<std::filesystem::path, size_t, Filesystem::PathHash>&
                textureCache) noexcept -> AssetImportResult2::Material
        {
            aiString aiAlbedoPath;
            material.GetTexture(aiTextureType_BASE_COLOR, 0, &aiAlbedoPath);
            std::filesystem::path albedoPath = aiAlbedoPath.C_Str();

            aiString aiNormalPath;
            material.GetTexture(aiTextureType_NORMALS, 0, &aiNormalPath);
            std::filesystem::path normalPath = aiNormalPath.C_Str();

            aiString aiMetallicPath;
            material.GetTexture(aiTextureType_METALNESS, 0, &aiMetallicPath);
            std::filesystem::path metallicPath = aiMetallicPath.C_Str();

            aiString aiRoughnessPath;
            material.GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &aiRoughnessPath);
            std::filesystem::path roughnessPath = aiRoughnessPath.C_Str();

            aiString aiAmbientOcclusionPath;
            material.GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aiAmbientOcclusionPath);
            std::filesystem::path ambientOcclusionPath = aiAmbientOcclusionPath.C_Str();

            aiString aiEmissivePath;
            material.GetTexture(aiTextureType_EMISSIVE, 0, &aiEmissivePath);
            std::filesystem::path emissivePath = aiEmissivePath.C_Str();

            aiColor3D aiAlbedoColor {1.0F, 1.0F, 1.0F};
            material.Get(AI_MATKEY_COLOR_DIFFUSE, aiAlbedoColor);

            aiColor3D aiEmissiveColor {0.0F, 0.0F, 0.0F};
            material.Get(AI_MATKEY_COLOR_EMISSIVE, aiEmissiveColor);

            AssetImportResult2::Material materialResult {};
            materialResult.albedoColor =
                glm::vec3(aiAlbedoColor.r, aiAlbedoColor.g, aiAlbedoColor.b);

            auto processTexture = [&](auto& textureIndex, auto& relativePath)
            {
                std::filesystem::path texturePath = assetDirectory / relativePath;
                if (std::filesystem::exists(texturePath))
                {
                    if (textureCache.contains(texturePath))
                    {
                        textureIndex = textureCache[texturePath];
                    }

                    else
                    {
                        textureIndex = textures.size();
                        textures.push_back(texturePath);
                        textureCache[texturePath] = textureIndex;
                    }
                }
                else
                {
                    textureIndex = std::numeric_limits<size_t>::max();
                }
            };

            processTexture(materialResult.albedoTextureIndex, albedoPath);
            processTexture(materialResult.normalTextureIndex, normalPath);
            processTexture(materialResult.metallicTextureIndex, metallicPath);
            processTexture(materialResult.roughnessTextureIndex, roughnessPath);
            processTexture(materialResult.aoTextureIndex, ambientOcclusionPath);
            processTexture(materialResult.emissiveTextureIndex, emissivePath);

            return materialResult;
        }

        [[nodiscard]] auto processMesh2(const aiMesh& mesh) noexcept
            -> AssetImportResult2::StaticMesh
        {
            AssetImportResult2::StaticMesh meshResult;
            meshResult.materialIndex = mesh.mMaterialIndex;

            meshResult.aabb.min =
                glm::vec3(mesh.mAABB.mMin.x, mesh.mAABB.mMin.y, mesh.mAABB.mMin.z);
            meshResult.aabb.max =
                glm::vec3(mesh.mAABB.mMax.x, mesh.mAABB.mMax.y, mesh.mAABB.mMax.z);

            std::vector<StaticMeshVertex> vertices;
            vertices.resize(mesh.mNumVertices);

            for (size_t i = 0; i < mesh.mNumVertices; i++)
            {
                StaticMeshVertex& vertex = vertices[i];

                vertex.position =
                    glm::vec3(mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z);

                if (mesh.HasNormals())
                {
                    vertex.normal =
                        glm::vec3(mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z);
                }

                if (mesh.HasTangentsAndBitangents())
                {
                    vertex.tangent =
                        glm::vec3(mesh.mTangents[i].x, mesh.mTangents[i].y, mesh.mTangents[i].z);
                    vertex.bitangent = glm::vec3(
                        mesh.mBitangents[i].x, mesh.mBitangents[i].y, mesh.mBitangents[i].z);
                }

                if (mesh.HasTextureCoords(0))
                {
                    vertex.uv = glm::vec2(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y);
                }
            }

            std::vector<uint32_t> indices;
            indices.resize(static_cast<size_t>(mesh.mNumFaces) * 3);

            for (size_t i = 0; i < mesh.mNumFaces; i++)
            {
                const aiFace& face = mesh.mFaces[i];

                indices[i * 3] = face.mIndices[0];
                indices[i * 3 + 1] = face.mIndices[1];
                indices[i * 3 + 2] = face.mIndices[2];
            }

            meshResult.vertices = std::move(vertices);
            meshResult.indices = std::move(indices);
            return meshResult;
        }

        [[nodiscard]] auto processNode2(const aiNode& node,
                                        std::vector<AssetImportResult2::Node>& nodes,
                                        size_t parent) noexcept -> std::vector<size_t>
        {
            Transform3D transform;
            // decompose aiMatrix4x4 into glm::vec3 and glm::quat
            const aiMatrix4x4& aiTransform = node.mTransformation;
            aiVector3D aiScale;
            aiQuaternion aiRotation;
            aiVector3D aiTranslation;

            aiTransform.Decompose(aiScale, aiRotation, aiTranslation);

            transform.scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);
            transform.rotation = glm::quat(aiRotation.w, aiRotation.x, aiRotation.y, aiRotation.z);
            transform.position = glm::vec3(aiTranslation.x, aiTranslation.y, aiTranslation.z);

            std::vector<size_t> nodes2;

            if (node.mNumMeshes > 0)
            {
                nodes2.reserve(node.mNumMeshes);

                for (size_t i = 0; i < node.mNumMeshes; i++)
                {
                    AssetImportResult2::Node nodeResult;
                    nodeResult.parentIndex = parent;
                    nodeResult.transform = transform;

                    nodeResult.meshIndex = node.mMeshes[i];

                    nodes2.push_back(nodes.size());

                    nodes.push_back(std::move(nodeResult));
                }
            }
            else
            {
                if (node.mNumChildren == 0)
                {
                    // If there are no meshes and no children, then this node is useless
                    return {};
                }

                nodes2.push_back(nodes.size());

                nodes.push_back(AssetImportResult2::Node {
                    .transform = transform,
                    .parentIndex = parent,
                });
            }

            std::vector<size_t> children;
            children.reserve(node.mNumChildren);

            for (size_t i = 0; i < node.mNumChildren; i++)
            {
                auto childNodes = processNode2(*node.mChildren[i], nodes, nodes2.back());
                children.insert(children.end(),
                                std::make_move_iterator(childNodes.begin()),
                                std::make_move_iterator(childNodes.end()));
            }

            nodes[nodes2.back()].childrenIndices = std::move(children);

            return nodes2;
        }

        [[nodiscard]] auto processScene2(const std::filesystem::path& assetPath,
                                         const aiScene& scene) noexcept -> AssetImportResult2
        {
            AssetImportResult2 result;

            std::unordered_map<std::filesystem::path, size_t, Filesystem::PathHash> textureCache {};

            std::filesystem::path assetDirectory = assetPath.parent_path();

            for (size_t i = 0; i < scene.mNumMaterials; ++i)
            {
                const auto* material = scene.mMaterials[i];

                result.materials.push_back(
                    processMaterial2(assetDirectory, *material, result.textures, textureCache));
            }

            for (size_t i = 0; i < scene.mNumMeshes; ++i)
            {
                const auto* mesh = scene.mMeshes[i];

                result.meshes.push_back(processMesh2(*mesh));
            }

            const auto* root = scene.mRootNode;

            if (root != nullptr)
            {
                result.rootNodes =
                    processNode2(*root, result.nodes, std::numeric_limits<size_t>::max());
            }

            return result;
        }

    }  // namespace

    auto importAsset2(const std::filesystem::path& assetPath) noexcept
        -> tl::expected<AssetImportResult2, AssetError>
    {
        if (!std::filesystem::exists(assetPath))
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        Assimp::Importer importer;

        constexpr auto importFlags = static_cast<unsigned int>(
            aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices
            | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

        const auto* scene = importer.ReadFile(assetPath.string(), importFlags);

        if (scene == nullptr)
        {
            std::cerr << "Failed to load asset: " << importer.GetErrorString() << std::endl;
            return tl::make_unexpected(FileFormatError {});
        }

        return processScene2(assetPath, *scene);
    }

    auto importTexture(const std::filesystem::path& texturePath) noexcept
        -> tl::expected<Texture, AssetError>
    {
        // Load using stb_image or ktx depending on file extension
        if (texturePath.extension() == ".ktx")
        {
            ktxTexture* ktxTexture = nullptr;
            ktxResult result = ktxTexture_CreateFromNamedFile(
                texturePath.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

            if (result != KTX_SUCCESS)
            {
                return tl::make_unexpected(FileFormatError {});
            }

            Texture texture;
            texture.type = Graphics::Texture::Type::e2D;
            texture.mips.resize(ktxTexture->numLevels);

            for (ktx_uint32_t i = 0; i < ktxTexture->numLevels; i++)
            {
                texture.mips[i].extent.x = std::max(ktxTexture->baseWidth >> i, 1U);
                texture.mips[i].extent.y = std::max(ktxTexture->baseHeight >> i, 1U);
                texture.mips[i].extent.z = 1;
                ktx_size_t offset = 0;
                ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
                texture.mips[i].offset = offset;
                texture.mips[i].size = ktxTexture_GetImageSize(ktxTexture, i);
            }

            texture.data = std::vector<std::byte>(ktxTexture_GetDataSize(ktxTexture));

            auto vkFormat = static_cast<vk::Format>(ktxTexture_GetVkFormat(ktxTexture));
            auto [channels, bitsPerChannel] = Graphics::vulkanFormatToChannelsAndBits(vkFormat);
            if (channels == 0 || bitsPerChannel == 0)
            {
                std::cout << "Unsupported format: " << vk::to_string(vkFormat) << std::endl;
                return tl::make_unexpected(FileFormatError {});
            }

            texture.channels = channels;
            texture.bitsPerChannel = bitsPerChannel;

            std::memcpy(texture.data.data(), ktxTexture_GetData(ktxTexture), texture.data.size());

            ktxTexture_Destroy(ktxTexture);

            return texture;
        }

        int width = 0;
        int height = 0;
        int channels = 0;
        stbi_uc* pixels = stbi_load(texturePath.string().c_str(), &width, &height, &channels, 0);

        if (pixels == nullptr)
        {
            return tl::make_unexpected(FileFormatError {});
        }

        if (channels == 3)
        {
            auto* newPixels = new stbi_uc[static_cast<size_t>(width) * height * 4];
            for (size_t i = 0; i < width * height; i++)
            {
                newPixels[i * 4] = pixels[i * 3];
                newPixels[i * 4 + 1] = pixels[i * 3 + 1];
                newPixels[i * 4 + 2] = pixels[i * 3 + 2];
                newPixels[i * 4 + 3] = 255;
            }

            stbi_image_free(pixels);
            pixels = newPixels;
            channels = 4;
        }

        Texture texture;
        texture.mips.resize(1);
        texture.mips[0].extent = glm::uvec3(width, height, 1);
        texture.mips[0].offset = 0;
        texture.mips[0].size = static_cast<size_t>(width) * height * channels;
        texture.data = std::vector<std::byte>(texture.mips[0].size);
        texture.channels = static_cast<uint8_t>(channels);
        texture.bitsPerChannel = 8;

        std::memcpy(texture.data.data(), pixels, texture.data.size());

        stbi_image_free(pixels);

        return texture;
    }

    auto saveTexture(Texture& texture) noexcept -> AssetFile
    {
        AssetFile assetFile;

        nlohmann::json json;
        json["dataType"] = "Texture";
        json["channels"] = texture.channels;
        json["bitsPerChannel"] = texture.bitsPerChannel;
        json["type"] = static_cast<uint32_t>(texture.type);
        json["mips"] = nlohmann::json::array();
        json["rawSize"] = texture.data.size();
        json["compression"] = "zstd";
        json["compressionLevel"] = ZSTD_defaultCLevel();

        for (size_t i = 0; i < texture.mips.size(); i++)
        {
            Texture::Mip& mip = texture.mips[i];
            json["mips"][i]["extent"] = mip.extent;
            json["mips"][i]["offset"] = mip.offset;
            json["mips"][i]["size"] = mip.size;
        }

        assetFile.json = json.dump();

        std::vector<char> binary;

        size_t compressedSize = ZSTD_compressBound(texture.data.size());
        binary.resize(compressedSize);

        compressedSize = ZSTD_compress(binary.data(),
                                       compressedSize,
                                       texture.data.data(),
                                       texture.data.size(),
                                       ZSTD_defaultCLevel());

        binary.resize(compressedSize);
        assetFile.binary = std::move(binary);

        return assetFile;
    }

    auto saveTexture(Texture& texture,
                     const std::filesystem::path& savePath,
                     const std::filesystem::path& prefix) noexcept -> tl::expected<void, AssetError>
    {
        const auto saveTexturePath = prefix / savePath;

        std::ofstream textureFile(saveTexturePath, std::ios::binary);
        if (!textureFile.is_open())
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        AssetFile assetFile = saveTexture(texture);
        saveAssetFile(textureFile, assetFile);

        texture.path = savePath;
        return {};
    }

    auto saveMaterial(Material& material) noexcept -> AssetFile
    {
        AssetFile assetFile;

        nlohmann::json json;
        json["dataType"] = "Material";
        json["albedoColor"] = material.albedoColor;
        json["emissiveColor"] = material.emissiveColor;
        json["metallicValue"] = material.metallicValue;
        json["roughnessValue"] = material.roughnessValue;
        json["albedoUseTexture"] = material.albedoUseTexture;
        json["normalUseTexture"] = material.normalUseTexture;
        json["metallicUseTexture"] = material.metallicUseTexture;
        json["roughnessUseTexture"] = material.roughnessUseTexture;
        json["occlusionUseTexture"] = material.occlusionUseTexture;
        json["emissiveUseTexture"] = material.emissiveUseTexture;
        json["albedoTexturePath"] = material.albedoTexturePath;
        json["normalTexturePath"] = material.normalTexturePath;
        json["metallicTexturePath"] = material.metallicTexturePath;
        json["roughnessTexturePath"] = material.roughnessTexturePath;
        json["occlusionTexturePath"] = material.occlusionTexturePath;
        json["emissiveTexturePath"] = material.emissiveTexturePath;

        assetFile.json = json.dump();

        return assetFile;
    }

    auto saveMaterial(Material& material,
                      const std::filesystem::path& savePath,
                      const std::filesystem::path& prefix) noexcept
        -> tl::expected<void, AssetError>
    {
        const auto saveMaterialPath = prefix / savePath;

        std::ofstream materialFile(saveMaterialPath, std::ios::binary);
        if (!materialFile.is_open())
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        AssetFile assetFile = saveMaterial(material);
        saveAssetFile(materialFile, assetFile);

        material.path = savePath;
        return {};
    }

    auto saveMesh(StaticMesh& mesh) noexcept -> AssetFile
    {
        AssetFile assetFile;

        nlohmann::json json;
        json["dataType"] = "StaticMesh";
        json["aabb"] = {
            {"min", mesh.aabb.min},
            {"max", mesh.aabb.max},
        };
        json["lods"] = nlohmann::json::array();

        for (size_t i = 0; i < mesh.lods.size(); i++)
        {
            MeshDetails& lod = mesh.lods[i];
            json["lods"][i]["vertexCount"] = lod.vertexCount;
            json["lods"][i]["indexCount"] = lod.indexCount;
            json["lods"][i]["vertexOffset"] = lod.vertexOffset;
            json["lods"][i]["indexOffset"] = lod.indexOffset;
        }

        json["materialPath"] = mesh.materialPath;

        json["compression"] = "zstd";
        json["compressionLevel"] = ZSTD_defaultCLevel();

        std::vector<char> binary;

        size_t vertexSize = sizeof(StaticMeshVertex) * mesh.vertices.size();
        size_t indexSize = sizeof(uint32_t) * mesh.indices.size();

        json["vertices"] = mesh.vertices.size();
        json["indices"] = mesh.indices.size();

        size_t vertexCompressedSize = ZSTD_compressBound(vertexSize);
        size_t indexCompressedSize = ZSTD_compressBound(indexSize);
        binary.resize(vertexCompressedSize + indexCompressedSize);

        vertexCompressedSize = ZSTD_compress(binary.data(),
                                             vertexCompressedSize,
                                             mesh.vertices.data(),
                                             vertexSize,
                                             ZSTD_defaultCLevel());

        indexCompressedSize = ZSTD_compress(binary.data() + vertexCompressedSize,
                                            indexCompressedSize,
                                            mesh.indices.data(),
                                            indexSize,
                                            ZSTD_defaultCLevel());

        binary.resize(vertexCompressedSize + indexCompressedSize);

        assetFile.binary = std::move(binary);

        json["vertexCompressedSize"] = vertexCompressedSize;
        json["indexCompressedSize"] = indexCompressedSize;

        assetFile.json = json.dump();

        return assetFile;
    }

    auto saveMesh(StaticMesh& mesh,
                  const std::filesystem::path& savePath,
                  const std::filesystem::path& prefix) noexcept -> tl::expected<void, AssetError>
    {
        const auto saveMeshPath = prefix / savePath;

        std::ofstream meshFile(saveMeshPath, std::ios::binary);
        if (!meshFile.is_open())
        {
            return tl::make_unexpected(FileNotFoundError {});
        }

        AssetFile assetFile = saveMesh(mesh);
        saveAssetFile(meshFile, assetFile);

        mesh.path = savePath;
        return {};
    }

    namespace
    {
        void createChildren(const AssetSceneImportInfo& info,
                            Scene& scene,
                            Entity parent,
                            std::span<const size_t> children,
                            std::span<const AssetImportResult2::Node> nodes)
        {
            for (const auto& child : children)
            {
                const auto& node = nodes[child];
                auto entity = scene.createEntity(parent);
                scene.addComponent<Transform3D>(entity, node.transform);

                GPUStaticMesh& mesh = info.meshes[node.meshIndex];
                StaticMeshComponent meshComponent = {.path = mesh.path, .pathHash = mesh.pathHash};
                scene.addComponent<StaticMeshComponent>(entity, meshComponent);

                createChildren(info, scene, entity, node.childrenIndices, nodes);
            }
        }
    }  // namespace

    void importScene(const AssetSceneImportInfo& info, Scene& scene, Entity parent) noexcept
    {
        createChildren(info, scene, parent, info.rootNodes, info.nodes);
    }

}  // namespace exage::Renderer
