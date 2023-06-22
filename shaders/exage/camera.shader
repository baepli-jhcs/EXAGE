#ifndef CAMERA_SHADER_H
#define CAMERA_SHADER_H

#include "bindless.shader"

struct Camera
{
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 position;
};

DEFINE_BINDLESS_STORAGE_BUFFER(readonly, Camera);

#endif // CAMERA_SHADER_H