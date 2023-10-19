#version 450

layout(push_constant) uniform PushConstant
{
    mat4 model;
    vec3 lightPos;
} pc;

layout(location = 0) in vec3 fragPos;

void main()
{
    float dist = length(fragPos - pc.lightPos);
    gl_FragDepth = dist;
}