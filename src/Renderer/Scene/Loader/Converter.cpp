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

#include "exage/Graphics/Texture.h"
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
        [[nodiscard]] auto processMaterial(
            const std::filesystem::path& assetDirectory,
            const aiMaterial& material,
            std::unordered_map<size_t, std::unique_ptr<Texture>>& textureCache) noexcept
            -> tl::expected<Material, AssetImportError>;

        [[nodiscard]] auto processMesh(
            const aiMesh& mesh, const std::unordered_map<size_t, Material*>& materialCache) noexcept
            -> tl::expected<Mesh, AssetImportError>;

        [[nodiscard]] auto processNode(const aiNode& node,
                                       const std::unordered_map<size_t, Mesh*>& meshCache,
                                       AssetImportResult::Node* parent) noexcept
            -> std::vector<std::unique_ptr<AssetImportResult::Node>>;

        [[nodiscard]] auto processScene(const std::filesystem::path& assetPath,
                                        const aiScene& scene) noexcept
            -> tl::expected<AssetImportResult, AssetImportError>
        {
            std::vector<std::unique_ptr<Material>> materials;
            materials.reserve(scene.mNumMaterials);

            std::unordered_map<size_t, std::unique_ptr<Texture>> textureCache;
            std::unordered_map<size_t, Material*> materialCache;

            std ::filesystem::path assetDirectory = assetPath.parent_path();

            for (size_t i = 0; i < scene.mNumMaterials; ++i)
            {
                const auto* material = scene.mMaterials[i];

                tl::expected materialReturn =
                    processMaterial(assetDirectory, *material, textureCache);

                if (materialReturn.has_value())
                {
                    auto materialPtr =
                        std::make_unique<Material>(std::move(materialReturn.value()));
                    materials.push_back(std::move(materialPtr));
                    materialCache[i] = materials.back().get();
                }

                else
                {
                    return tl::make_unexpected(materialReturn.error());
                }
            }

            std::vector<std::unique_ptr<Mesh>> meshes;
            meshes.reserve(scene.mNumMeshes);

            std::unordered_map<size_t, Mesh*> meshCache;

            for (size_t i = 0; i < scene.mNumMeshes; ++i)
            {
                const auto* mesh = scene.mMeshes[i];
                auto meshReturn = processMesh(*mesh, materialCache);

                if (meshReturn.has_value())
                {
                    auto meshPtr = std::make_unique<Mesh>(std::move(meshReturn.value()));
                    meshes.push_back(std::move(meshPtr));
                    meshCache[i] = meshes.back().get();
                }

                else
                {
                    return tl::make_unexpected(meshReturn.error());
                }
            }

            AssetImportResult result;
            result.materials = std::move(materials);
            result.meshes = std::move(meshes);

            result.textures.reserve(textureCache.size());

            for (auto& [path, texture] : textureCache)
            {
                result.textures.push_back(std::move(texture));
            }

            const auto* root = scene.mRootNode;

            if (root != nullptr)
            {
                result.rootNodes = processNode(*root, meshCache, nullptr);
            }

            return result;
        }

        [[nodiscard]] auto processMaterial(
            const std::filesystem::path& assetDirectory,
            const aiMaterial& material,
            std::unordered_map<size_t, std::unique_ptr<Texture>>& textureCache) noexcept
            -> tl::expected<Material, AssetImportError>
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

            AlbedoInfo albedoInfo;
            albedoInfo.color = glm::vec3(aiAlbedoColor.r, aiAlbedoColor.g, aiAlbedoColor.b);
            albedoInfo.useTexture = !albedoPath.empty();

            NormalInfo normalInfo;
            normalInfo.useTexture = !normalPath.empty();

            MetallicInfo metallicInfo;
            metallicInfo.useTexture = !metallicPath.empty();

            RoughnessInfo roughnessInfo;
            roughnessInfo.useTexture = !roughnessPath.empty();

            OcclusionInfo occlusionInfo;
            occlusionInfo.useTexture = !ambientOcclusionPath.empty();

            EmissiveInfo emissiveInfo;
            emissiveInfo.color = glm::vec3(aiEmissiveColor.r, aiEmissiveColor.g, aiEmissiveColor.b);
            emissiveInfo.useTexture = !emissivePath.empty();

            // Refactor
            auto processTexture = [&](auto& textureInfo, auto& relativePath)
            {
                std::filesystem::path texturePath = assetDirectory / relativePath;
                if (textureInfo.useTexture)
                {
                    size_t textureHash = std::filesystem::hash_value(texturePath);
                    if (textureCache.contains(textureHash))
                    {
                        textureInfo.texture = textureCache[textureHash].get();
                    }

                    else
                    {
                        tl::expected textureReturn = importTexture(texturePath);

                        if (textureReturn.has_value())
                        {
                            auto texturePtr =
                                std::make_unique<Texture>(std::move(textureReturn.value()));
                            textureCache[textureHash] = std::move(texturePtr);
                            textureInfo.texture = textureCache[textureHash].get();
                        }
                        else
                        {
                            textureInfo.useTexture = false;
                        }
                    }
                }
            };

            processTexture(albedoInfo, albedoPath);
            processTexture(normalInfo, normalPath);
            processTexture(metallicInfo, metallicPath);
            processTexture(roughnessInfo, roughnessPath);
            processTexture(occlusionInfo, ambientOcclusionPath);
            processTexture(emissiveInfo, emissivePath);

            Material materialResult {};
            materialResult.albedo = albedoInfo;
            materialResult.normal = normalInfo;
            materialResult.metallic = metallicInfo;
            materialResult.roughness = roughnessInfo;
            materialResult.occlusion = occlusionInfo;
            materialResult.emissive = emissiveInfo;

            return materialResult;
        }

        [[nodiscard]] auto processMesh(
            const aiMesh& mesh, const std::unordered_map<size_t, Material*>& materialCache) noexcept
            -> tl::expected<Mesh, AssetImportError>
        {
            Mesh meshReturn;

            meshReturn.aabb.min =
                glm::vec3(mesh.mAABB.mMin.x, mesh.mAABB.mMin.y, mesh.mAABB.mMin.z);
            meshReturn.aabb.max =
                glm::vec3(mesh.mAABB.mMax.x, mesh.mAABB.mMax.y, mesh.mAABB.mMax.z);

            std::vector<MeshVertex> vertices;
            vertices.resize(mesh.mNumVertices);

            for (size_t i = 0; i < mesh.mNumVertices; i++)
            {
                MeshVertex& vertex = vertices[i];

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

            MeshLOD lod;
            lod.vertices = std::move(vertices);
            lod.indices = std::move(indices);

            meshReturn.lods.push_back(std::move(lod));

            // Find material using index
            auto materialIt = materialCache.find(mesh.mMaterialIndex);
            if (materialIt != materialCache.end())
            {
                meshReturn.material = materialIt->second;
            }

            return meshReturn;
        }

        [[nodiscard]] auto processNode(const aiNode& node,
                                       const std::unordered_map<size_t, Mesh*>& meshCache,
                                       AssetImportResult::Node* parent) noexcept
            -> std::vector<std::unique_ptr<AssetImportResult::Node>>
        {
            Transform3D transform;
            // decompose aiMatrix4x4 into glm::vec3 and glm::quat
            const aiMatrix4x4& aiTransform = node.mTransformation;

            transform.position = glm::vec3(aiTransform.a4, aiTransform.b4, aiTransform.c4);

            // Now, length of x, y, z is scale, where x is first col
            glm::vec3 xVec = glm::vec3(aiTransform.a1, aiTransform.b1, aiTransform.c1);
            glm::vec3 yVec = glm::vec3(aiTransform.a2, aiTransform.b2, aiTransform.c2);
            glm::vec3 zVec = glm::vec3(aiTransform.a3, aiTransform.b3, aiTransform.c3);

            transform.scale = glm::vec3(glm::length(xVec), glm::length(yVec), glm::length(zVec));

            // Now, normalize x, y, z
            xVec = glm::normalize(xVec);
            yVec = glm::normalize(yVec);
            zVec = glm::normalize(zVec);

            // Now, calculate rotation
            glm::mat3 rotationMatrix = glm::mat3(xVec, yVec, zVec);
            transform.rotation = glm::quat_cast(rotationMatrix);

            std::vector<std::unique_ptr<AssetImportResult::Node>> nodes;

            if (node.mNumMeshes > 0)
            {
                nodes.reserve(node.mNumMeshes);

                for (size_t i = 0; i < node.mNumMeshes; i++)
                {
                    AssetImportResult::Node nodeResult;
                    nodeResult.parent = parent;
                    nodeResult.transform = transform;

                    nodeResult.meshIndex = node.mMeshes[i];

                    nodes.push_back(
                        std::make_unique<AssetImportResult::Node>(std::move(nodeResult)));
                }
            }
            else
            {
                if (node.mNumChildren == 0)
                {
                    // If there are no meshes and no children, then this node is useless
                    return {};
                }

                nodes.push_back(std::make_unique<AssetImportResult::Node>(AssetImportResult::Node {
                    .transform = transform,
                    .parent = parent,
                }));
            }

            std::vector<std::unique_ptr<AssetImportResult::Node>> children;
            children.reserve(node.mNumChildren);

            for (size_t i = 0; i < node.mNumChildren; i++)
            {
                auto childNodes = processNode(*node.mChildren[i], meshCache, nodes.back().get());
                children.insert(children.end(),
                                std::make_move_iterator(childNodes.begin()),
                                std::make_move_iterator(childNodes.end()));
            }

            nodes.back()->children = std::move(children);

            return nodes;
        }

    }  // namespace

    auto importAsset(const std::filesystem::path& assetPath) noexcept
        -> tl::expected<AssetImportResult, AssetImportError>
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

        return processScene(assetPath, *scene);
    }

    auto importTexture(const std::filesystem::path& texturePath) noexcept
        -> tl::expected<Texture, TextureImportError>
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
            std::optional<Graphics::Format> format = Graphics::toExageFormat(vkFormat);
            if (!format)
            {
                std::cout << "Unsupported format: " << vk::to_string(vkFormat) << std::endl;
                return tl::make_unexpected(FileFormatError {});
            }
            texture.format = *format;

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

        Texture texture;
        texture.mips.resize(1);
        texture.mips[0].extent = glm::uvec3(width, height, 1);
        texture.mips[0].offset = 0;
        texture.mips[0].size = static_cast<size_t>(width) * height * channels;
        texture.data = std::vector<std::byte>(texture.mips[0].size);

        switch (channels)
        {
            case 1:
                texture.format = Graphics::Format::eR8;
                break;
            case 2:
                texture.format = Graphics::Format::eRG8;
                break;
            case 3:
                texture.format = Graphics::Format::eRGB8;
                break;
            case 4:
                texture.format = Graphics::Format::eRGBA8;
                break;

            default:
                std::cout << "Unsupported number of channels: " << channels << std::endl;
                return tl::make_unexpected(FileFormatError {});
        }

        std::memcpy(texture.data.data(), pixels, texture.data.size());

        stbi_image_free(pixels);

        return texture;
    }

    auto saveAssets(const AssetImportResult& assets,
                    const std::filesystem::path& saveDirectory,
                    const std::filesystem::path& prefix) noexcept -> std::optional<DirectoryError>
    {
        const auto texturesDirectory = saveDirectory / "textures";
        const auto materialsDirectory = saveDirectory / "materials";
        const auto meshesDirectory = saveDirectory / "meshes";

        const auto saveTexturesDir = prefix / texturesDirectory;
        const auto saveMaterialsDir = prefix / materialsDirectory;
        const auto saveMeshesDir = prefix / meshesDirectory;

        std::filesystem::create_directories(saveTexturesDir);
        std::filesystem::create_directories(saveMaterialsDir);
        std::filesystem::create_directories(saveMeshesDir);

        if (!std::filesystem::exists(texturesDirectory)
            || !std::filesystem::exists(materialsDirectory)
            || !std::filesystem::exists(meshesDirectory))
        {
            return DirectoryError {};
        }

        for (size_t i = 0; i < assets.textures.size(); i++)
        {
            auto& texture = *assets.textures[i];
            const auto texturePath =
                texturesDirectory / (std::to_string(i).append(TEXTURE_EXTENSION));

            std::ofstream textureFile(texturePath, std::ios::binary);
            cereal::BinaryOutputArchive archive(textureFile);
            archive(texture);

            texture.path = texturePath.string();
        }

        for (size_t i = 0; i < assets.materials.size(); i++)
        {
            auto& material = *assets.materials[i];
            const auto materialPath =
                materialsDirectory / (std::to_string(i).append(MATERIAL_EXTENSION));

            if (material.albedo.texture != nullptr)
            {
                material.albedo.texturePath = material.albedo.texture->path;
            }

            if (material.normal.texture != nullptr)
            {
                material.normal.texturePath = material.normal.texture->path;
            }

            if (material.metallic.texture != nullptr)
            {
                material.metallic.texturePath = material.metallic.texture->path;
            }

            if (material.roughness.texture != nullptr)
            {
                material.roughness.texturePath = material.roughness.texture->path;
            }

            if (material.occlusion.texture != nullptr)
            {
                material.occlusion.texturePath = material.occlusion.texture->path;
            }

            if (material.emissive.texture != nullptr)
            {
                material.emissive.texturePath = material.emissive.texture->path;
            }

            std::ofstream materialFile(materialPath, std::ios::binary);
            cereal::BinaryOutputArchive archive(materialFile);
            archive(material);

            material.path = materialPath.string();
        }

        for (size_t i = 0; i < assets.meshes.size(); i++)
        {
            auto& mesh = *assets.meshes[i];
            const auto meshPath = meshesDirectory / (std::to_string(i).append(MESH_EXTENSION));

            if (mesh.material != nullptr)
            {
                mesh.materialPath = mesh.material->path;
            }

            std::ofstream meshFile(meshPath, std::ios::binary);
            cereal::BinaryOutputArchive archive(meshFile);
            archive(mesh);

            mesh.path = meshPath.string();
        }

        return std::nullopt;
    }

    auto saveTexture(Texture& texture,
                     const std::filesystem::path& savePath,
                     const std::filesystem::path& prefix) noexcept -> std::optional<SaveError>
    {
        const auto saveTexturePath = prefix / savePath;

        std::ofstream textureFile(saveTexturePath, std::ios::binary);
        cereal::BinaryOutputArchive archive(textureFile);
        archive(texture);

        texture.path = saveTexturePath.string();

        return std::nullopt;
    }

    namespace
    {
        void createChildren(const AssetSceneImportInfo& info,
                            Scene& scene,
                            Entity parent,
                            std::span<std::unique_ptr<AssetImportResult::Node>> children)
        {
            for (const auto& child : children)
            {
                auto entity = scene.createEntity(parent);
                scene.addComponent<Transform3D>(entity, child->transform);

                GPUMesh& mesh = info.meshes[child->meshIndex];
                scene.addComponent<GPUMesh>(entity, mesh);

                createChildren(info, scene, entity, child->children);
            }
        }
    }  // namespace

    void importScene(const AssetSceneImportInfo& info, Scene& scene, Entity parent) noexcept
    {
        createChildren(info, scene, parent, info.rootNodes);
    }

}  // namespace exage::Renderer
