#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../bindless.shader"
#include "../camera.shader"

struct Transform
{
    mat4 model;
    mat4 normal;
    mat4 modelViewProjection;
};

DEFINE_BINDLESS_STORAGE_BUFFER(readonly, Transform);

// push constants
layout(push_constant) uniform PushConstant
{
    uint cameraIndex;
    uint transformIndex;
    uint materialIndex;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;

void main()
{
    Transform transform = GetBindlessStorageBuffer(Transform, pc.transformIndex);
    Camera camera = GetBindlessStorageBuffer(Camera, pc.cameraIndex);

    vec4 worldPosition = transform.model * vec4(position, 1.0);
    worldPos = worldPosition.xyz;

    fragNormal = normalize(transform.normal * vec4(normal, 0.0)).xyz;
    fragUV = uv;    
    fragTangent = normalize(transform.normal * vec4(tangent, 0.0)).xyz;
    fragBitangent = normalize(transform.normal * vec4(bitangent, 0.0)).xyz;

    gl_Position = camera.viewProjection * worldPosition;

}
