#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../bindless.shader"
    
struct Material 
{
    vec3 albedoColor;
    vec3 emissiveColor;

    float metallicValue;
    float roughnessValue;

    bool albedoUseTexture;
    bool normalUseTexture;
    bool metallicUseTexture;
    bool roughnessUseTexture;
    bool occlusionUseTexture;
    bool emissiveUseTexture;

    uint albedoTextureIndex;
    uint normalTextureIndex;
    uint metallicTextureIndex;
    uint roughnessTextureIndex;
    uint occlusionTextureIndex;
    uint emissiveTextureIndex;
};

layout(push_constant) uniform PushConstant
{
    uint cameraIndex;
    uint transformIndex;
    uint materialIndex;
    uint samplerIndex;
} pc;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out float gMetallic;
layout(location = 4) out float gRoughness;
layout(location = 5) out float gAO;
layout(location = 6) out vec4 gEmissive;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

DEFINE_BINDLESS_SAMPLED_TEXTURES();
DEFINE_BINDLESS_SAMPLERS();

DEFINE_BINDLESS_STORAGE_BUFFER(readonly, Material);

void main()
{
    gPosition = vec4(worldPos, 1.0);

    Material material = GetBindlessStorageBuffer(Material, pc.materialIndex);
    
    if (material.normalUseTexture)
    {
        vec3 normalMap = SampleBindless2DTexture(pc.samplerIndex, material.normalTextureIndex, uv).xyz;
        normalMap = normalize(normalMap * 2.0 - 1.0);
        normalMap = normalize(tangent * normalMap.x + bitangent * normalMap.y + normalMap.z * normal);
        gNormal = vec4(normalMap, 1.0);
    }
    else
    {
        gNormal = vec4(normal, 1.0);
    }

    if (material.albedoUseTexture)
    {
        gAlbedo = vec4(SampleBindless2DTexture(pc.samplerIndex, material.albedoTextureIndex, uv).xyz, 1.0);
    }
    else
    {
        gAlbedo = vec4(material.albedoColor, 1.0);
    }

    if (material.metallicUseTexture)
    {
        gMetallic = SampleBindless2DTexture(pc.samplerIndex, material.metallicTextureIndex, uv).x;
    }
    else
    {
        gMetallic = material.metallicValue;
    }

    if (material.roughnessUseTexture)
    {
        gRoughness = SampleBindless2DTexture(pc.samplerIndex, material.roughnessTextureIndex, uv).x;
    }
    else
    {
        gRoughness = material.roughnessValue;
    }

    if (material.occlusionUseTexture)
    {
        gAO = SampleBindless2DTexture(pc.samplerIndex, material.occlusionTextureIndex, uv).x;
    }
    else
    {
        gAO = 1.0;
    }

    if (material.emissiveUseTexture)
    {
        gEmissive = vec4(SampleBindless2DTexture(pc.samplerIndex, material.emissiveTextureIndex, uv).xyz, 1.0);
    }
    else
    {
        gEmissive = vec4(material.emissiveColor, 1.0);
    }
}