#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "exage/Renderer/Scene/Loader/Converter.h"

#include <FreeImage.h>
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <exage/utils/serialization.h>
// #include <ktx.h>
// #include <gli/format.hpp>
// #include <gli/gli.hpp>
// #include <stb_image.h>
#include <fp16.h>
#include <fp16/fp16.h>
#include <tl/expected.hpp>
#include <zstd.h>

#include "exage/Core/Errors.h"
#include "exage/Filesystem/Directories.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Loader/AssetFile.h"
#include "exage/Renderer/Scene/Loader/Loader.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/platform/Vulkan/VulkanUtils.h"
// #include "ktxvulkan.h"

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
                glm::vec4(mesh.mAABB.mMin.x, mesh.mAABB.mMin.y, mesh.mAABB.mMin.z, 1.0F);
            meshResult.aabb.max =
                glm::vec4(mesh.mAABB.mMax.x, mesh.mAABB.mMax.y, mesh.mAABB.mMax.z, 1.0F);

            std::vector<StaticMeshVertex> vertices;
            vertices.resize(mesh.mNumVertices);

            for (size_t i = 0; i < mesh.mNumVertices; i++)
            {
                StaticMeshVertex& vertex = vertices[i];

                vertex.position =
                    glm::vec4(mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z, 1.0F);

                if (mesh.HasNormals())
                {
                    vertex.normal =
                        glm::vec4(mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z, 0.0F);
                }

                if (mesh.HasTangentsAndBitangents())
                {
                    vertex.tangent = glm::vec4(
                        mesh.mTangents[i].x, mesh.mTangents[i].y, mesh.mTangents[i].z, 0.0F);
                    vertex.bitangent = glm::vec4(
                        mesh.mBitangents[i].x, mesh.mBitangents[i].y, mesh.mBitangents[i].z, 0.0F);
                }

                if (mesh.HasTextureCoords(0))
                {
                    vertex.uv = glm::vec4(
                        mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y, 0.0F, 0.0F);
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
        -> tl::expected<AssetImportResult2, Error>
    {
        if (!std::filesystem::exists(assetPath))
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }

        Assimp::Importer importer;

        constexpr auto importFlags = static_cast<unsigned int>(
            aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices
            | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

        const auto* scene = importer.ReadFile(assetPath.string(), importFlags);

        if (scene == nullptr)
        {
            std::cerr << "Failed to load asset: " << importer.GetErrorString() << std::endl;
            return tl::make_unexpected(Errors::FileFormat {});
        }

        return processScene2(assetPath, *scene);
    }

    auto importTexture(const std::filesystem::path& texturePath) noexcept
        -> tl::expected<Texture, Error>
    {
        // Load using stb_image or ktx depending on file extension
        // if (texturePath.extension() == ".ktx")
        // {
        //     ktxTexture* ktxTexture = nullptr;
        //     ktxResult result = ktxTexture_CreateFromNamedFile(
        //         texturePath.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        //         &ktxTexture);

        //     if (result != KTX_SUCCESS)
        //     {
        //         return tl::make_unexpected(Errors::FileFormat {});
        //     }

        //     Texture texture;
        //     texture.type = Graphics::Texture::Type::e2D;
        //     texture.mips.resize(ktxTexture->numLevels);

        //     for (ktx_uint32_t i = 0; i < ktxTexture->numLevels; i++)
        //     {
        //         texture.mips[i].extent.x = std::max(ktxTexture->baseWidth >> i, 1U);
        //         texture.mips[i].extent.y = std::max(ktxTexture->baseHeight >> i, 1U);
        //         texture.mips[i].extent.z = 1;
        //         ktx_size_t offset = 0;
        //         ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
        //         texture.mips[i].offset = offset;
        //         texture.mips[i].size = ktxTexture_GetImageSize(ktxTexture, i);
        //     }

        //     texture.data = std::vector<std::byte>(ktxTexture_GetDataSize(ktxTexture));

        //     auto vkFormat = static_cast<vk::Format>(ktxTexture_GetVkFormat(ktxTexture));
        //     auto [channels, bitsPerChannel] = Graphics::vulkanFormatToChannelsAndBits(vkFormat);
        //     if (channels == 0 || bitsPerChannel == 0)
        //     {
        //         std::cout << "Unsupported format: " << vk::to_string(vkFormat) << std::endl;
        //         return tl::make_unexpected(Errors::FileFormat {});
        //     }

        //     texture.channels = channels;
        //     texture.bitsPerChannel = bitsPerChannel;

        //     std::memcpy(texture.data.data(), ktxTexture_GetData(ktxTexture),
        //     texture.data.size());

        //     ktxTexture_Destroy(ktxTexture);

        //     return texture;
        // }

        // std::string extension = texturePath.extension().string();
        // if (extension == ".ktx" || extension == ".dds" || extension == ".kmg"
        //     || extension == ".ktx2")
        // {
        //     // Load using gli
        //     gli::texture texture = gli::load(texturePath.string());
        //     if (texture.empty())
        //     {
        //         return tl::make_unexpected(Errors::FileFormat {});
        //     }

        //     auto bitsPerPixel = gli::detail::bits_per_pixel(texture.format());
        //     auto channels = gli::component_count(texture.format());
        //     auto bitsPerChannel = bitsPerPixel / channels;

        //     uint8_t bpcAfter = 0;

        //     // Handle compressed formats by reencoding
        //     switch (bitsPerChannel)
        //     {
        //         case 8:
        //         {
        //             texture = gli::convert(texture, gli::FORMAT_RGBA8_UNORM_PACK8);
        //             bpcAfter = 8;
        //             break;
        //         }
        //         case 16:
        //         {
        //             texture = gli::convert(texture, gli::FORMAT_RGBA16_SFLOAT_PACK16);
        //             bpcAfter = 16;
        //             break;
        //         }
        //         case 32:
        //         {
        //             texture = gli::convert(texture, gli::FORMAT_RGBA16_SFLOAT_PACK16);
        //             bpcAfter = 16;
        //             break;
        //         }

        //         default:
        //         {
        //             return tl::make_unexpected(Errors::FileFormat {});
        //         }
        //     }

        //     Texture result;
        //     result.channels = 4;
        //     result.bitsPerChannel = bpcAfter;
        //     result.data = std::vector<std::byte>(texture.size());
        //     result.layers = texture.layers();

        //     switch (texture.target())
        //     {
        //         case gli::TARGET_1D:
        //         {
        //             result.type = Graphics::Texture::Type::e1D;
        //             break;
        //         }
        //         case gli::TARGET_1D_ARRAY:
        //         {
        //             result.type = Graphics::Texture::Type::e1D;
        //             break;
        //         }
        //         case gli::TARGET_2D:
        //         {
        //             result.type = Graphics::Texture::Type::e2D;
        //             break;
        //         }
        //         case gli::TARGET_2D_ARRAY:
        //         {
        //             result.type = Graphics::Texture::Type::e3D;
        //             break;
        //         }
        //         case gli::TARGET_3D:
        //         {
        //             result.type = Graphics::Texture::Type::e3D;
        //             break;
        //         }
        //         case gli::TARGET_CUBE:
        //         {
        //             result.type = Graphics::Texture::Type::eCube;
        //             break;
        //         }
        //         case gli::TARGET_CUBE_ARRAY:
        //         {
        //             result.type = Graphics::Texture::Type::eCube;
        //             break;
        //         }
        //         default:
        //         {
        //             return tl::make_unexpected(Errors::FileFormat {});
        //         }
        //     }

        //     size_t offset = 0;
        //     for (size_t i = 0; i < texture.levels(); i++)
        //     {
        //         result.mips[i].extent = texture.extent(i);
        //         result.mips[i].offset = offset;
        //         result.mips[i].size = texture.size(i);

        //         offset += texture.size(i);
        //     }

        //     std::memcpy(result.data.data(), texture.data(), result.data.size());

        //     return result;
        // }

        // int width = 0;
        // int height = 0;
        // int channels = 0;
        // stbi_uc* pixels = stbi_load(texturePath.string().c_str(), &width, &height, &channels, 0);

        // if (pixels == nullptr)
        // {
        //     return tl::make_unexpected(Errors::FileFormat {});
        // }

        // if (channels == 3)
        // {
        //     auto* newPixels = new stbi_uc[static_cast<size_t>(width) * height * 4];
        //     for (size_t i = 0; i < width * height; i++)
        //     {
        //         newPixels[i * 4] = pixels[i * 3];
        //         newPixels[i * 4 + 1] = pixels[i * 3 + 1];
        //         newPixels[i * 4 + 2] = pixels[i * 3 + 2];
        //         newPixels[i * 4 + 3] = 255;
        //     }

        //     stbi_image_free(pixels);
        //     pixels = newPixels;
        //     channels = 4;
        // }

        // Texture texture;
        // texture.mips.resize(1);
        // texture.mips[0].extent = glm::uvec3(width, height, 1);
        // texture.mips[0].offset = 0;
        // texture.mips[0].size = static_cast<size_t>(width) * height * channels;
        // texture.data = std::vector<std::byte>(texture.mips[0].size);
        // texture.channels = static_cast<uint8_t>(channels);
        // texture.bitsPerChannel = 8;
        // texture.type = Graphics::Texture::Type::e2D;
        // texture.layers = 1;

        // std::memcpy(texture.data.data(), pixels, texture.data.size());

        // stbi_image_free(pixels);

        // return texture;

        // FreeImage
        // fipImage image {};
        // bool success = image.load(texturePath.string().c_str());
        // if (!success)
        // {
        //     return tl::make_unexpected(Errors::FileFormat {});
        // }

        // Texture texture;
        // texture.mips.resize(1);
        // texture.mips[0].extent = glm::uvec3(image.getWidth(), image.getHeight(), 1);
        // texture.mips[0].offset = 0;

        // auto imageType = image.getImageType();
        // switch (imageType)
        // {
        //     case FIT_BITMAP:
        //     {
        //         texture.mips[0].size =
        //             static_cast<size_t>(image.getWidth()) * image.getHeight() * 4;
        //         texture.channels = 4;
        //         texture.bitsPerChannel = 8;
        //         break;
        //     }
        //     case FIT_RGB16:
        //     case FIT_RGBA16:
        //     case FIT_RGBF:
        //     {
        //         bool converted = image.convertToRGBF();
        //         if (!converted)
        //         {
        //             return tl::make_unexpected(Errors::FileFormat {});
        //         }
        //     }
        //     case FIT_RGBAF:
        //     {
        //         texture.mips[0].size =
        //             static_cast<size_t>(image.getWidth()) * image.getHeight() * 4 *
        //             sizeof(float);
        //         texture.channels = 4;
        //         texture.bitsPerChannel = 32;
        //         break;
        //     }
        //     case FIT_UINT16:
        //     case FIT_INT16:
        //     case FIT_UINT32:
        //     case FIT_INT32:
        //     case FIT_DOUBLE:
        //     {
        //         bool converted = image.convertToFloat();
        //         if (!converted)
        //         {
        //             return tl::make_unexpected(Errors::FileFormat {});
        //         }
        //     }
        //     case FIT_FLOAT:
        //     {
        //         texture.mips[0].size =
        //             static_cast<size_t>(image.getWidth()) * image.getHeight() * sizeof(float);
        //         texture.channels = 1;
        //         texture.bitsPerChannel = 32;
        //         break;
        //     }

        //     default:
        //     {
        //         return tl::make_unexpected(Errors::FileFormat {});
        //     }
        // }

        // texture.data = std::vector<std::byte>(image.getImageMemorySize());
        // std::memcpy(texture.data.data(), image.accessPixels(), texture.data.size());

        // texture.type = Graphics::Texture::Type::e2D;
        // texture.layers = 1;

        // return texture;

        FREE_IMAGE_FORMAT type = FreeImage_GetFileType(texturePath.string().c_str());

        FIBITMAP* image = FreeImage_Load(type, texturePath.string().c_str(), 0);
        if (image == nullptr)
        {
            return tl::make_unexpected(Errors::FileFormat {});
        }

        Texture texture;
        texture.mips.resize(1);
        texture.mips[0].extent =
            glm::uvec3(FreeImage_GetWidth(image), FreeImage_GetHeight(image), 1);
        texture.mips[0].offset = 0;

        // Get the image type
        FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(image);
        switch (imageType)
        {
            case FIT_BITMAP:
            {
                // Convert to 32-bit RGBA format
                FIBITMAP* temp = FreeImage_ConvertTo32Bits(image);
                FreeImage_Unload(image);
                image = temp;

                texture.mips[0].size =
                    static_cast<size_t>(FreeImage_GetWidth(image)) * FreeImage_GetHeight(image) * 4;
                texture.channels = 4;
                texture.bitsPerChannel = 8;
                break;
            }
            case FIT_RGB16:
            case FIT_RGBA16:
            case FIT_RGBF:
            {
                // Convert to 128-bit RGBAF format
                FIBITMAP* temp = FreeImage_ConvertToRGBAF(image);
                FreeImage_Unload(image);
                image = temp;
            }
            case FIT_RGBAF:
            {
                texture.mips[0].size = static_cast<size_t>(FreeImage_GetWidth(image))
                    * FreeImage_GetHeight(image) * 4 * sizeof(float);
                texture.channels = 4;
                texture.bitsPerChannel = 32;
                break;
            }
            case FIT_UINT16:
            case FIT_INT16:
            case FIT_UINT32:
            case FIT_INT32:
            case FIT_DOUBLE:
            {
                // Convert to 32-bit float format
                FIBITMAP* temp = FreeImage_ConvertToFloat(image);
                FreeImage_Unload(image);
                image = temp;
            }
            case FIT_FLOAT:
            {
                texture.mips[0].size = static_cast<size_t>(FreeImage_GetWidth(image))
                    * FreeImage_GetHeight(image) * sizeof(float);
                texture.channels = 1;
                texture.bitsPerChannel = 32;
                break;
            }

            default:
            {
                return tl::make_unexpected(Errors::FileFormat {});
            }
        }

        auto size = texture.mips[0].size;

        // Copy the pixel data to the texture
        texture.data = std::vector<std::byte>(size);
        // std::memcpy(texture.data.data(), FreeImage_GetBits(image), texture.data.size());

        // if type is FIT_BITMAP, don't copy directly, but get each pixel individually since
        // freeimage stores in either BGRA or RGBA format
        if (imageType == FIT_BITMAP)
        {
            for (size_t i = 0; i < FreeImage_GetWidth(image); i++)
            {
                for (size_t j = 0; j < FreeImage_GetHeight(image); j++)
                {
                    RGBQUAD color;
                    FreeImage_GetPixelColor(image, i, j, &color);

                    texture.data[(j * FreeImage_GetWidth(image) + i) * 4] =
                        static_cast<std::byte>(color.rgbRed);
                    texture.data[(j * FreeImage_GetWidth(image) + i) * 4 + 1] =
                        static_cast<std::byte>(color.rgbGreen);
                    texture.data[(j * FreeImage_GetWidth(image) + i) * 4 + 2] =
                        static_cast<std::byte>(color.rgbBlue);
                    texture.data[(j * FreeImage_GetWidth(image) + i) * 4 + 3] =
                        static_cast<std::byte>(color.rgbReserved);
                }
            }
        }
        else
        {
            std::memcpy(texture.data.data(), FreeImage_GetBits(image), texture.data.size());
        }

        // Unload the image
        FreeImage_Unload(image);

        texture.type = Graphics::Texture::Type::e2D;
        texture.layers = 1;

        return texture;
    }

    auto saveTexture(Texture& texture) noexcept -> AssetFile
    {
        AssetFile assetFile;

        nlohmann::json json;
        json["dataType"] = "Texture";
        json["path"] = texture.path;
        json["channels"] = texture.channels;
        json["bitsPerChannel"] = texture.bitsPerChannel;
        json["layers"] = texture.layers;
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

    void optimizePrecision(Texture& texture) noexcept
    {
        // Convert 32 bit floats to 16 bit floats
        if (!(texture.bitsPerChannel == 32))
        {
            return;
        }

        std::span<uint16_t> newPixels =
            std::span(reinterpret_cast<uint16_t*>(texture.data.data()), texture.data.size() / 4);
        std::span<const float> oldPixels = std::span(reinterpret_cast<float*>(texture.data.data()),
                                                     texture.data.size() / sizeof(float));

        for (size_t i = 0; i < oldPixels.size(); i++)
        {
            newPixels[i] = fp16_ieee_from_fp32_value(oldPixels[i]);
        }

        texture.bitsPerChannel = 16;
        texture.data.resize(texture.data.size() / 2);
    }

    auto saveTexture(Texture& texture, const std::filesystem::path& savePath) noexcept
        -> tl::expected<void, Error>
    {
        std::ofstream textureFile(savePath, std::ios::binary);
        if (!textureFile.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }

        AssetFile assetFile = saveTexture(texture);
        saveAssetFile(textureFile, assetFile);

        return {};
    }

    auto saveMaterial(Material& material) noexcept -> AssetFile
    {
        AssetFile assetFile;

        nlohmann::json json;
        json["dataType"] = "Material";
        json["path"] = material.path;
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

    auto saveMaterial(Material& material, const std::filesystem::path& savePath) noexcept
        -> tl::expected<void, Error>
    {
        std::ofstream materialFile(savePath, std::ios::binary);
        if (!materialFile.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }

        AssetFile assetFile = saveMaterial(material);
        saveAssetFile(materialFile, assetFile);

        return {};
    }

    auto saveMesh(StaticMesh& mesh) noexcept -> AssetFile
    {
        AssetFile assetFile;

        nlohmann::json json;
        json["dataType"] = "StaticMesh";
        json["path"] = mesh.path;
        json["aabb"] = {
            {"min", mesh.aabb.min},
            {"max", mesh.aabb.max},
        };
        json["lods"] = nlohmann::json::array();

        for (size_t i = 0; i < mesh.lodCount; i++)
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

    auto saveMesh(StaticMesh& mesh, const std::filesystem::path& savePath) noexcept
        -> tl::expected<void, Error>
    {
        std::ofstream meshFile(savePath, std::ios::binary);
        if (!meshFile.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {});
        }

        AssetFile assetFile = saveMesh(mesh);
        saveAssetFile(meshFile, assetFile);

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
