#version 450

layout(push_constant) uniform PushConstant
{
    mat4 modelViewProjection;
    vec3 lightPos;
    float lightRadius;
    uint layer;
} pc;

layout(location = 0) in vec3 fragPos;

void main()
{
    float dist = length(fragPos - pc.lightPos);
    gl_FragDepth = dist;
}