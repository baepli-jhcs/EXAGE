#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <entt/core/hashed_string.hpp>
#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/Buffer.h"
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
        glm::vec3 min {};
        glm::vec3 max {};
    };

    struct Mesh
    {
        std::filesystem::path path;

        std::vector<MeshDetails> lods;

        std::filesystem::path materialPath;

        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;

        AABB aabb;
    };

    struct GPUMesh
    {
        std::filesystem::path path;
        size_t pathHash;

        std::vector<MeshDetails> lods;

        std::filesystem::path materialPath;
        GPUMaterial material;

        std::shared_ptr<Graphics::Buffer> vertexBuffer;
        std::shared_ptr<Graphics::Buffer> indexBuffer;

        AABB aabb;
    };

    struct MeshComponent
    {
        std::filesystem::path path;
        size_t pathHash;
    };

    constexpr std::string_view MESH_EXTENSION = ".exmesh";
}  // namespace exage::Renderer
