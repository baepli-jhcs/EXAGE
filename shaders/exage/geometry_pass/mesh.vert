#version 450

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "../bindless.shader"

struct Camera
{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec3 position;
};

DEFINE_BINDLESS_STORAGE_BUFFER(readonly, Camera);

struct Transform
{
    mat4 model;
    mat4 normal;
    mat4 modelViewProjection;
};

DEFINE_BINDLESS_STORAGE_BUFFER(readonly, Transform);

layout (std430, binding = BINDLESS_STORAGE_BUFFER_BINDING) readonly buffer TransformTest
{
    mat4 model;
    mat4 normal;
    mat4 modelViewProjection;
} transformTest[];

layout(std430, binding = BINDLESS_STORAGE_BUFFER_BINDING) readonly buffer CameraTest
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	vec3 position;
} cameraTest[];

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

    gl_Position = camera.viewProj * worldPosition;

    debugPrintfEXT("CameraIndex = %d", pc.cameraIndex);
    debugPrintfEXT("TransformIndex = %d", pc.transformIndex);
    debugPrintfEXT("Camera = %v3f", camera.position);

    debugPrintfEXT("CameraTest = %v3f", cameraTest[pc.cameraIndex].position);

    debugPrintfEXT("Transform model matrix row 0 = %v4f", transform.model[0]);
    debugPrintfEXT("Transform model matrix row 1 = %v4f", transform.model[1]);
    debugPrintfEXT("Transform model matrix row 2 = %v4f", transform.model[2]);
    debugPrintfEXT("Transform model matrix row 3 = %v4f", transform.model[3]);
    
    debugPrintfEXT("Transform normal matrix row 0 = %v4f", transform.normal[0]);
    debugPrintfEXT("Transform normal matrix row 1 = %v4f", transform.normal[1]);
    debugPrintfEXT("Transform normal matrix row 2 = %v4f", transform.normal[2]);
    debugPrintfEXT("Transform normal matrix row 3 = %v4f", transform.normal[3]);

    debugPrintfEXT("position = %v3f", position);
    debugPrintfEXT("normal = %v3f", normal);
    debugPrintfEXT("uv = %v2f", uv);
    debugPrintfEXT("tangent = %v3f", tangent);
    debugPrintfEXT("bitangent = %v3f", bitangent);
    debugPrintfEXT("worldPosition = %v4f", worldPosition);
    debugPrintfEXT("gl_Position = %v4f", gl_Position);
}
