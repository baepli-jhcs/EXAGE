﻿#include "exage/Renderer/Scene/Loader/Converter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>

namespace exage::Renderer
{
    namespace
    {
        // General process: First, process every material in the scene. Save each to
        // options.saveDirectory. Do the same for all meshes, connecting them to the materials. If
        // options.scene is not nullptr, traverse the node hierarchy recusively. At every node,
        // create a new entity with the associated mesh and transform.
        [[nodiscard]] auto processMaterial(const aiMaterial& material,
                                           const AssetImportOptions& options,
                                           size_t index) noexcept -> Material;
        void processMesh(const aiMesh& mesh, const AssetImportOptions& options) noexcept;

        void processNode(const aiNode& node,
                         Entity parent,
                         const AssetImportOptions& options) noexcept;

        void processScene(const aiScene& scene, const AssetImportOptions& options) noexcept
        {
            std::vector<Material> materials;
            materials.reserve(scene.mNumMaterials);

            for (size_t i = 0; i < scene.mNumMaterials; ++i)
            {
                const auto* material = scene.mMaterials[i];
                materials.push_back(processMaterial(*material, options, i));
            }

            for (size_t i = 0; i < scene.mNumMeshes; ++i)
            {
                const auto* mesh = scene.mMeshes[i];
                processMesh(*mesh, options);
            }

            if (options.scene != nullptr)
            {
                const auto* root = scene.mRootNode;

                if (root != nullptr)
                {
                    processNode(*root, options.parent, options);
                }
            }
        }

        [[nodiscard]] auto processMaterial(const aiMaterial& material,
                                           const AssetImportOptions& options,
                                           size_t index) noexcept -> Material
        {
            std::filesystem::path savePath =
                options.saveDirectory / "material" / (std::to_string(index) + ".exm");

            if (options.overwriteExisting || !std::filesystem::exists(savePath))
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

                aiColor3D aiAlbedoColor {1.0f, 1.0f, 1.0f};
                material.Get(AI_MATKEY_COLOR_DIFFUSE, aiAlbedoColor);

                aiColor3D aiEmissiveColor {0.0f, 0.0f, 0.0f};
                material.Get(AI_MATKEY_COLOR_EMISSIVE, aiEmissiveColor);

                AlbedoInfo albedoInfo;
                albedoInfo.texturePath = albedoPath.string();
                albedoInfo.color = glm::vec3(aiAlbedoColor.r, aiAlbedoColor.g, aiAlbedoColor.b);
                albedoInfo.useTexture = !albedoPath.empty();

                NormalInfo normalInfo;
                normalInfo.texturePath = normalPath.string();
                normalInfo.useTexture = !normalPath.empty();

                MetallicInfo metallicInfo;
                metallicInfo.texturePath = metallicPath.string();
                metallicInfo.useTexture = !metallicPath.empty();

                RoughnessInfo roughnessInfo;
                roughnessInfo.texturePath = roughnessPath.string();
                roughnessInfo.useTexture = !roughnessPath.empty();

                OcclusionInfo occlusionInfo;
                occlusionInfo.texturePath = ambientOcclusionPath.string();
                occlusionInfo.useTexture = !ambientOcclusionPath.empty();

                EmissiveInfo emissiveInfo;
                emissiveInfo.texturePath = emissivePath.string();
                emissiveInfo.color =
                    glm::vec3(aiEmissiveColor.r, aiEmissiveColor.g, aiEmissiveColor.b);
                emissiveInfo.useTexture = !emissivePath.empty();

                TextureImportOptions textureOptions;
                textureOptions.overwriteExisting = options.overwriteExisting;
                textureOptions.commandBuffer = options.commandBuffer;
                textureOptions.resourceManager = options.resourceManager;
                textureOptions.layout = options.layout;
                textureOptions.access = options.access;
                textureOptions.pipelineStage = options.pipelineStage;

                if (albedoInfo.useTexture)
                {
                    textureOptions.texturePath = options.assetPath.parent_path() / albedoPath;
                    textureOptions.savePath = options.saveDirectory / "texture" / albedoPath;
                    albedoInfo.texture = importTexture(textureOptions);
                }
                if (normalInfo.useTexture)
                {
                    textureOptions.texturePath = options.assetPath.parent_path() / normalPath;
                    textureOptions.savePath = options.saveDirectory / "texture" / normalPath;
                    normalInfo.texture = importTexture(textureOptions);
                }
                if (metallicInfo.useTexture)
                {
                    textureOptions.texturePath = options.assetPath.parent_path() / metallicPath;
                    textureOptions.savePath = options.saveDirectory / "texture" / metallicPath;
                    metallicInfo.texture = importTexture(textureOptions);
                }
                if (roughnessInfo.useTexture)
                {
                    textureOptions.texturePath = options.assetPath.parent_path() / roughnessPath;
                    textureOptions.savePath = options.saveDirectory / "texture" / roughnessPath;
                    roughnessInfo.texture = importTexture(textureOptions);
                }
                if (occlusionInfo.useTexture)
                {
                    textureOptions.texturePath =
                        options.assetPath.parent_path() / ambientOcclusionPath;
                    textureOptions.savePath =
                        options.saveDirectory / "texture" / ambientOcclusionPath;
                    occlusionInfo.texture = importTexture(textureOptions);
                }
                if (emissiveInfo.useTexture)
                {
                    textureOptions.texturePath = options.assetPath.parent_path() / emissivePath;
                    textureOptions.savePath = options.saveDirectory / "texture" / emissivePath;
                    emissiveInfo.texture = importTexture(textureOptions);
                }

                Material material {};
                material.albedo = albedoInfo;
                material.normal = normalInfo;
                material.metallic = metallicInfo;
                material.roughness = roughnessInfo;
                material.occlusion = occlusionInfo;
                material.emissive = emissiveInfo;
                material.path = savePath.string();

                // Save with cereal

                std::ofstream fileStream {savePath, std::ios::binary};
                cereal::BinaryOutputArchive archive {fileStream};
                archive(material);

                return material;
            }
        }
    }  // namespace

    auto importAsset(const AssetImportOptions& options) noexcept -> std::optional<AssetImportError>
    {
        if (!std::filesystem::exists(options.assetPath))
        {
            return FileNotFoundError {};
        }

        if (!std::filesystem::exists(options.saveDirectory))
        {
            return SaveDirectoryError {};
        }

        Assimp::Importer importer;

        constexpr auto importFlags = aiProcess_Triangulate | aiProcess_CalcTangentSpace
            | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_GenUVCoords
            | aiProcess_GenBoundingBoxes;

        const auto* scene = importer.ReadFile(options.assetPath.string(), importFlags);

        if (scene == nullptr)
        {
            return FileFormatError {importer.GetErrorString()};
        }

        processScene(*scene, options);

        return std::nullopt;
    }
}  // namespace exage::Renderer
