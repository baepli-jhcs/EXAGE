#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../bindless.shader"

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    vec3 lightPos;
    uint transformIndex;
} pc;

struct PointLightTransforms
{
    mat4 viewProjections[6];
};

DEFINE_BINDLESS_STORAGE_BUFFER(readonly, PointLightTransforms);

layout(location = 0) out vec4 fragPos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            fragPos = gl_in[i].gl_Position;
            gl_Position = GetBindlessStorageBuffer(PointLightTransforms, pc.transformIndex).viewProjections[face] * fragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  