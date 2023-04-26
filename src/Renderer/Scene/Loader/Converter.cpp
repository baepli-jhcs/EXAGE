#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "exage/Renderer/Scene/Loader/Converter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>

#include "assimp/matrix4x4.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Loader/Errors.h"
#include "exage/Renderer/Scene/Loader/Loader.h"
#include "exage/Scene/Hierarchy.h"
#include "tl/expected.hpp"

namespace exage::Renderer
{
    namespace
    {
        [[nodiscard]] auto processMaterial(
            const aiMaterial& material,
            std::unordered_map<std::filesystem::path, std::unique_ptr<Texture>>&
                textureCache) noexcept -> tl::expected<Material, AssetImportError>;

        [[nodiscard]] auto processMesh(
            const aiMesh& mesh, const std::unordered_map<size_t, Material*>& materialCache) noexcept
            -> tl::expected<Mesh, AssetImportError>;

        [[nodiscard]] auto processNode(const aiNode& node,
                                       const std::unordered_map<size_t, Mesh*>& meshCache,
                                       AssetImportResult::Node* parent) noexcept
            -> std::vector<std::unique_ptr<AssetImportResult::Node>>;

        [[nodiscard]] auto processScene(const aiScene& scene) noexcept
            -> tl::expected<AssetImportResult, AssetImportError>
        {
            std::vector<std::unique_ptr<Material>> materials;
            materials.reserve(scene.mNumMaterials);

            std::unordered_map<std::filesystem::path, std::unique_ptr<Texture>> textureCache;
            std::unordered_map<size_t, Material*> materialCache;

            for (size_t i = 0; i < scene.mNumMaterials; ++i)
            {
                const auto* material = scene.mMaterials[i];

                tl::expected materialReturn = processMaterial(*material, textureCache);

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

            result.textures.resize(textureCache.size());

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
            const aiMaterial& material,
            std::unordered_map<std::filesystem::path, std::unique_ptr<Texture>>&
                textureCache) noexcept -> tl::expected<Material, AssetImportError>
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
            auto processTexture = [&](auto& textureInfo, auto& texturePath)
            {
                if (textureInfo.useTexture)
                {
                    if (textureCache.contains(texturePath))
                    {
                        textureInfo.texture = textureCache[texturePath].get();
                    }

                    else
                    {
                        tl::expected textureReturn = importTexture(texturePath);

                        if (textureReturn.has_value())
                        {
                            auto texturePtr =
                                std::make_unique<Texture>(std::move(textureReturn.value()));
                            textureCache[texturePath] = std::move(texturePtr);
                            textureInfo.texture = textureCache[texturePath].get();
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

                    auto meshIt = meshCache.find(node.mMeshes[i]);
                    nodeResult.mesh = meshIt->second;

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

            return children;
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

        constexpr auto importFlags = aiProcess_Triangulate | aiProcess_CalcTangentSpace
            | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_GenUVCoords
            | aiProcess_GenBoundingBoxes;

        const auto* scene = importer.ReadFile(assetPath.string(), importFlags);

        if (scene == nullptr)
        {
            std::cerr << "Failed to load asset: " << importer.GetErrorString() << std::endl;
            return tl::make_unexpected(FileFormatError {});
        }

        return processScene(*scene);
    }

    auto importTexture(const std::filesystem::path& texturePath) noexcept
        -> tl::expected<Texture, TextureImportError>
    {
        // Load using stb_image or ktx depending on file extension
        if (texturePath.extension() == ".ktx")
        {
        }
        else
        {
        }
    }
}  // namespace exage::Renderer
