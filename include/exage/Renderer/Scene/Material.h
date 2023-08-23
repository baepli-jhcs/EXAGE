#pragma once

#include <filesystem>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <stdint.h>

#include "exage/Core/Core.h"
#include "exage/Graphics/Texture.h"

namespace exage::Renderer
{
    struct Texture
    {
        struct Mip
        {
            glm::uvec3 extent;
            uint64_t offset;
            uint64_t size;
        };

        std::string path;
        std::vector<Mip> mips;
        std::vector<std::byte> data;
        uint8_t channels;
        uint8_t bitsPerChannel;
        uint8_t layers;
        Graphics::Texture::Type type;
    };

    struct GPUTexture
    {
        std::string path;

        std::shared_ptr<Graphics::Texture> texture;
    };

    constexpr std::string_view TEXTURE_EXTENSION = ".extex";

    struct Material
    {
        std::string path;

        glm::vec3 albedoColor = glm::vec3(1.0f);
        glm::vec3 emissiveColor = glm::vec3(0.0f);

        float metallicValue = 0.0f;
        float roughnessValue = 0.0f;

        bool albedoUseTexture = false;
        bool normalUseTexture = false;
        bool metallicUseTexture = false;
        bool roughnessUseTexture = false;
        bool occlusionUseTexture = false;
        bool emissiveUseTexture = false;

        std::string albedoTexturePath;
        std::string normalTexturePath;
        std::string metallicTexturePath;
        std::string roughnessTexturePath;
        std::string occlusionTexturePath;
        std::string emissiveTexturePath;
    };

    struct GPUMaterial
    {
        struct Data
        {
            alignas(16) glm::vec3 albedoColor = glm::vec3(1.0f);
            alignas(16) glm::vec3 emissiveColor = glm::vec3(0.0f);

            alignas(4) float metallicValue = 0.0f;
            alignas(4) float roughnessValue = 0.0f;

            alignas(4) bool albedoUseTexture = false;
            alignas(4) bool normalUseTexture = false;
            alignas(4) bool metallicUseTexture = false;
            alignas(4) bool roughnessUseTexture = false;
            alignas(4) bool occlusionUseTexture = false;
            alignas(4) bool emissiveUseTexture = false;

            alignas(4) uint32_t albedoTextureIndex = 0;
            alignas(4) uint32_t normalTextureIndex = 0;
            alignas(4) uint32_t metallicTextureIndex = 0;
            alignas(4) uint32_t roughnessTextureIndex = 0;
            alignas(4) uint32_t occlusionTextureIndex = 0;
            alignas(4) uint32_t emissiveTextureIndex = 0;
        };

        std::string path;

        GPUTexture albedoTexture;
        GPUTexture emissiveTexture;
        GPUTexture normalTexture;
        GPUTexture metallicTexture;
        GPUTexture roughnessTexture;
        GPUTexture occlusionTexture;

        std::shared_ptr<Graphics::Buffer> buffer;
    };

    inline auto materialDataFromGPUAndCPU(const GPUMaterial& gpu, const Material& material)
        -> GPUMaterial::Data
    {
        GPUMaterial::Data data;

        data.albedoColor = material.albedoColor;
        data.emissiveColor = material.emissiveColor;

        data.metallicValue = material.metallicValue;
        data.roughnessValue = material.roughnessValue;

        data.albedoUseTexture = material.albedoUseTexture;
        data.normalUseTexture = material.normalUseTexture;
        data.metallicUseTexture = material.metallicUseTexture;
        data.roughnessUseTexture = material.roughnessUseTexture;
        data.occlusionUseTexture = material.occlusionUseTexture;
        data.emissiveUseTexture = material.emissiveUseTexture;

        if (gpu.albedoTexture.texture)
        {
            data.albedoTextureIndex = gpu.albedoTexture.texture->getBindlessID().id;
        }
        if (gpu.emissiveTexture.texture)
        {
            data.emissiveTextureIndex = gpu.emissiveTexture.texture->getBindlessID().id;
        }
        if (gpu.normalTexture.texture)
        {
            data.normalTextureIndex = gpu.normalTexture.texture->getBindlessID().id;
        }
        if (gpu.metallicTexture.texture)
        {
            data.metallicTextureIndex = gpu.metallicTexture.texture->getBindlessID().id;
        }
        if (gpu.roughnessTexture.texture)
        {
            data.roughnessTextureIndex = gpu.roughnessTexture.texture->getBindlessID().id;
        }
        if (gpu.occlusionTexture.texture)
        {
            data.occlusionTextureIndex = gpu.occlusionTexture.texture->getBindlessID().id;
        }

        return data;
    }

    constexpr std::string_view MATERIAL_EXTENSION = ".exmat";
}  // namespace exage::Renderer
