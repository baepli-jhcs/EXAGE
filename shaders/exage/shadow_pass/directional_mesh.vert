#version 450

#extension GL_ARB_shader_viewport_layer_array : enable

layout(push_constant) uniform PushConstant
{
    mat4 modelViewProjection;
    int layer;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

void main()
{
    gl_Position = pc.modelViewProjection * vec4(position, 1.0);
    gl_Layer = pc.layer;
}