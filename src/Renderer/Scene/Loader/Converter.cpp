#include <fstream>

#include "exage/Renderer/Scene/Loader/Converter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>

#include "exage/Renderer/Scene/Loader/Loader.h"

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
                                           size_t index) noexcept
            -> tl::expected<Material, AssetImportError>;

        [[nodiscard]] auto processMesh(const aiMesh& mesh,
                                       const AssetImportOptions& options,
                                       size_t index) noexcept
            -> tl::expected<Mesh, AssetImportError>;

        void processNode(const aiNode& node,
                         Entity parent,
                         const AssetImportOptions& options) noexcept;

        [[nodiscard]] auto processScene(const aiScene& scene,
                                        const AssetImportOptions& options) noexcept
            -> std::optional<AssetImportError>
        {
            std::vector<Material> materials;
            materials.reserve(scene.mNumMaterials);

            std::filesystem::path materialPath = options.saveDirectory / "materials";
            std::filesystem::create_directories(materialPath);

            for (size_t i = 0; i < scene.mNumMaterials; ++i)
            {
                const auto* material = scene.mMaterials[i];

                tl::expected materialReturn = processMaterial(*material, options, i);

                if (materialReturn.has_value())
                {
                    materials.push_back(std::move(materialReturn.value()));
                }

                else
                {
                    return materialReturn.error();
                }
            }

            std::vector<Mesh> meshes;

            std::filesystem::path meshPath = options.saveDirectory / "meshes";
            std::filesystem::create_directories(meshPath);

            for (size_t i = 0; i < scene.mNumMeshes; ++i)
            {
                const auto* mesh = scene.mMeshes[i];
                auto meshReturn = processMesh(*mesh, options, i);

                if (meshReturn.has_value())
                {
                    meshes.push_back(std::move(meshReturn.value()));
                }

                else
                {
                    return meshReturn.error();
                }
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
                                           size_t index) noexcept
            -> tl::expected<Material, AssetImportError>
        {
            std::filesystem::path savePath = options.saveDirectory / "materials"
                / (std::to_string(index).append(MATERIAL_EXTENSION));

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

                std::ofstream fileStream {savePath, std::ios::binary};
                cereal::BinaryOutputArchive archive {fileStream};
                archive(material);

                return material;
            }

            MaterialLoadOptions loadOptions;
            loadOptions.path = savePath;

            tl::expected materialReturn = loadMaterial(loadOptions);

            if (materialReturn.has_value())
            {
                return materialReturn.value();
            }
            else
            {
                AssetImportError error = std::visit(
                    [](auto&& value) { return AssetImportError(value); }, materialReturn.error());

                return tl::make_unexpected(error);
            }
        }

        [[nodiscard]] auto processMesh(const aiMesh& mesh,
                                       const AssetImportOptions& options,
                                       size_t index) noexcept
            -> tl::expected<Mesh, AssetImportError>
        {
            std::filesystem::path savePath =
                options.saveDirectory / "meshes" / (std::to_string(index).append(MESH_EXTENSION));

            if (options.overwriteExisting || !std::filesystem::exists(savePath))
            {
                Mesh meshReturn;

                meshReturn.path = savePath.string();
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
                        vertex.tangent = glm::vec3(
                            mesh.mTangents[i].x, mesh.mTangents[i].y, mesh.mTangents[i].z);
                        vertex.bitangent = glm::vec3(
                            mesh.mBitangents[i].x, mesh.mBitangents[i].y, mesh.mBitangents[i].z);
                    }

                    if (mesh.HasTextureCoords(0))
                    {
                        vertex.uv =
                            glm::vec2(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y);
                    }
                }

                std::vector<uint32_t> indices;
                indices.resize(mesh.mNumFaces * 3);

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

                std::ofstream fileStream {savePath, std::ios::binary};
                cereal::BinaryOutputArchive archive {fileStream};
                archive(meshReturn);

                return meshReturn;
            }

            if (options.cache)
            {
                return *options.cache->getMesh(savePath);
            }

            MeshLoadOptions loadOptions;
            loadOptions.path = savePath;
            loadOptions.commandBuffer = options.commandBuffer;
            loadOptions.access = options.access;
            loadOptions.pipelineStage = options.pipelineStage;

            tl::expected meshReturn = loadMesh(loadOptions);

            if (meshReturn.has_value())
            {
                return meshReturn.value();
            }
            else
            {
                AssetImportError error = std::visit(
                    [](auto&& value) { return AssetImportError(value); }, meshReturn.error());

                return tl::make_unexpected(error);
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
            std::cerr << "Failed to load asset: " << importer.GetErrorString() << std::endl;
            return FileFormatError {};
        }

        return processScene(*scene, options);
    }
}  // namespace exage::Renderer
