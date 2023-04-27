#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Renderer/Scene/Material.h"

namespace exage::Renderer
{
    struct MeshVertex
    {
        glm::vec3 position {};
        glm::vec3 normal {};
        glm::vec2 uv {};
        glm::vec3 tangent {};
        glm::vec3 bitangent {};

        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(position, normal, uv, tangent, bitangent);
        }
    };

    struct MeshLOD
    {
        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;

        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(vertices, indices);
        }
    };

    struct AABB
    {
        glm::vec3 min {};
        glm::vec3 max {};
        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(min, max);
        }
    };

    struct Mesh
    {
        std::string path;  // Not filesystem::path because of serialization

        std::vector<MeshLOD> lods;

        std::string materialPath;
        Material* material = nullptr;

        AABB aabb;

        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(path, lods, materialPath, aabb);
        }
    };

    struct GPUMeshDetails
    {
        uint32_t vertexCount;
        uint32_t indexCount;

        size_t vertexOffset;
        size_t indexOffset;
    };

    struct GPUMesh
    {
        std::string path;

        std::vector<GPUMeshDetails> lods;

        std::string materialPath;
        GPUMaterial material;

        AABB aabb;

        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(path, materialPath, aabb);
        }
    };

    constexpr std::string_view MESH_EXTENSION = ".exmesh";
}  // namespace exage::Renderer
