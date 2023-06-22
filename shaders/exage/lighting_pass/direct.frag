#version 450

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "../bindless.shader"
#include "../camera.shader"

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

DEFINE_BINDLESS_SAMPLED_TEXTURES();
DEFINE_BINDLESS_SAMPLERS();

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
    float physicalRadius;
    float attenuationRadius;
    float shadowBias;
    int shadowMapIndex;
};

DEFINE_BINDLESS_STORAGE_BUFFER_WITH_MEMBERS(readonly, PointLightArray, {
    uint count;
    PointLight pointLights[];
});

#define MAX_CASCADE_LEVELS 5

struct DirectionalLight
{
    mat4 cascadeViewProjections[MAX_CASCADE_LEVELS];
    vec3 direction;
    float intensity;
    vec3 color;
    int shadowMapIndex;
    float shadowBias;
    uint cascadeLevels;
    float cascadeSplits[MAX_CASCADE_LEVELS];
};

DEFINE_BINDLESS_STORAGE_BUFFER_WITH_MEMBERS(readonly, DirectionalLightArray, {
    uint count;
    DirectionalLight directionalLights[];
});

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float physicalRadius;
    float innerCutoff;
    float outerCutoff;
    int shadowMapIndex;
    float shadowBias;
};

DEFINE_BINDLESS_STORAGE_BUFFER_WITH_MEMBERS(readonly, SpotLightArray, {
    uint count;
    SpotLight spotLights[];
});

layout(push_constant) uniform PushConstant
{
    uint pointLightBufferIndex;
    uint directionalLightBufferIndex;
    uint spotLightBufferIndex;
    uint positionTextureIndex;
    uint normalTextureIndex;
    uint albedoTextureIndex;
    uint metallicTextureIndex;
    uint roughnessTextureIndex;
    uint occlusionTextureIndex;
    uint emissiveTextureIndex;
    uint cameraBufferIndex;
    uint samplerIndex;
} pc;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / max(denom, 0.0001); // Prevent divide by zero for roughness=0.0
}

float geometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float pointShadowCalculation(vec3 position, PointLight light)
{
    return 1.0; // TODO: Implement
}

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

float directionalShadowCalculation(vec3 position, vec3 viewSpacePosition, vec3 normal, DirectionalLight light)
{
    if (light.shadowMapIndex == -1)
    {
        return 1.0;
    }

    return 1.0; // TODO: Implement

    // vec3 lightDirection = normalize(-light.direction);

    // uint cascadeIndex = 0;
    // for (uint i = 0; i < light.cascadeLevels - 1; i++)
    // {
    //     if (viewSpacePosition.z < light.cascadeSplits[i])
    //     {
    //         cascadeIndex = i + 1;
    //         break;
    //     }
    // }

    // mat4 shadowMatrix = biasMat * light.cascadeViewProjections[cascadeIndex];
    // vec4 shadowCoord = shadowMatrix * vec4(position, 1.0);

    // float shadow = 1.0;
    // if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
    // {
	// 	float dist = SampleBindless2DArrayTexture(pc.samplerIndex, light.shadowMapIndex, vec3(shadowCoord.xy, cascadeIndex)).r;
	// 	if (shadowCoord.w > 0 && dist < shadowCoord.z - light.shadowBias)
    //     {
    //         shadow = 0.0;
    //     }
	// }

    // return shadow;
}

void main()
{
    vec3 position = SampleBindless2DTexture(pc.samplerIndex, pc.positionTextureIndex, uv).xyz;
    vec3 normal = SampleBindless2DTexture(pc.samplerIndex, pc.normalTextureIndex, uv).xyz;
    vec3 albedo = SampleBindless2DTexture(pc.samplerIndex, pc.albedoTextureIndex, uv).xyz;
    float metallic = SampleBindless2DTexture(pc.samplerIndex, pc.metallicTextureIndex, uv).x;
    float roughness = SampleBindless2DTexture(pc.samplerIndex, pc.roughnessTextureIndex, uv).x;
    float occlusion = SampleBindless2DTexture(pc.samplerIndex, pc.occlusionTextureIndex, uv).x;
    vec3 emissive = SampleBindless2DTexture(pc.samplerIndex, pc.emissiveTextureIndex, uv).xyz;

    Camera camera = GetBindlessStorageBuffer(Camera, pc.cameraBufferIndex);
    vec4 viewPosition = camera.view * vec4(position, 1.0);

    vec3 N = normalize(normal);
    vec3 V = normalize(camera.position - position);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (uint i = 0; i < GetBindlessStorageBufferWithMembers(PointLightArray, pc.pointLightBufferIndex).count; i++)
    {
        PointLight light = GetBindlessStorageBufferWithMembers(PointLightArray, pc.pointLightBufferIndex).pointLights[i];

        // // Shadow
        float shadow = pointShadowCalculation(position, light);

        vec3 L = normalize(light.position - position);
        vec3 H = normalize(V + L);
        
        float distance = length(light.position - position);
        float radiusSq = light.physicalRadius * light.physicalRadius;
        float distanceSq = distance * distance;
        float attenuation = 2 / (distanceSq + radiusSq + distance * sqrt(radiusSq + distanceSq));
        vec3 radiance = light.color * light.intensity * attenuation;

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = nominator / max(denominator, 0.001);

        float NdotL = max(dot(N, L), 0.0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 diffuse = (kD * albedo) / PI;

        Lo += (diffuse + specular) * radiance * NdotL * shadow;
    }

    for (uint i = 0; i < GetBindlessStorageBufferWithMembers(DirectionalLightArray, pc.directionalLightBufferIndex).count; i++)
    {
        DirectionalLight light = GetBindlessStorageBufferWithMembers(DirectionalLightArray, pc.directionalLightBufferIndex).directionalLights[i];

        // Shadow
        float shadow = directionalShadowCalculation(position, viewPosition.xyz, N, light);

        vec3 L = normalize(-light.direction);
        vec3 H = normalize(V + L);
        
        vec3 radiance = light.color * light.intensity;

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = nominator / max(denominator, 0.001);

        float NdotL = max(dot(N, L), 0.0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 diffuse = (kD * albedo) / PI;

        Lo += (diffuse + specular) * radiance * NdotL * shadow;
    }

    vec3 color = Lo + emissive;

    outColor = vec4(color, 1.0);
}
