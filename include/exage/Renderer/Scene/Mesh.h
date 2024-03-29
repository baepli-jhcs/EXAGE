﻿#pragma once

#include <entt/core/hashed_string.hpp>
#include <exage/utils/serialization.h>
#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Renderer/Scene/Material.h"

namespace exage::Renderer
{
    constexpr uint32_t MAX_LOD_COUNT = 8;

    struct StaticMeshVertex
    {
        glm::vec4 position {};
        glm::vec4 normal {};
        glm::vec4 uv {};
        glm::vec4 tangent {};
        glm::vec4 bitangent {};
    };

    struct MeshDetails
    {
        uint32_t vertexCount;
        uint32_t indexCount;

        uint32_t vertexOffset;
        uint32_t indexOffset;
    };

    struct AABB
    {
        glm::vec4 min {};
        glm::vec4 max {};
    };

    struct StaticMesh
    {
        std::string path;

        uint32_t lodCount;
        std::array<MeshDetails, MAX_LOD_COUNT> lods;

        std::string materialPath;

        std::vector<StaticMeshVertex> vertices;
        std::vector<uint32_t> indices;

        AABB aabb;
    };

    struct GPUStaticMesh
    {
        std::string path;
        size_t pathHash;

        uint32_t lodCount;
        std::array<MeshDetails, MAX_LOD_COUNT> lods;

        std::string materialPath;
        GPUMaterial material;

        AABB aabb;
    };

    struct StaticMeshComponent
    {
        std::string path;
        size_t pathHash;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(path, pathHash);
        }
    };

    constexpr std::string_view MESH_EXTENSION = ".exmesh";
}  // namespace exage::Renderer
