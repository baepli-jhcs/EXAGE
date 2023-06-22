#ifndef BINDLESS_SHADER_H
#define BINDLESS_SHADER_H

#extension GL_EXT_nonuniform_qualifier : enable

#define BINDLESS_STORAGE_BUFFER_BINDING 0
#define BINDLESS_SAMPLER_BINDING 1
#define BINDLESS_SAMPLED_TEXTURE_BINDING 2
#define BINDLESS_STORAGE_TEXTURE_BINDING 3

#define EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName) uExage##StructName

#define DEFINE_BINDLESS_SAMPLED_TEXTURES() \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform texture2D exage2DTextures[]; \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform textureCube exageCubeTextures[]; \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform texture3D exage3DTextures[]; \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform texture2DArray exage2DArrayTextures[]

#define DEFINE_BINDLESS_SAMPLERS() \
    layout (binding = BINDLESS_SAMPLER_BINDING) uniform sampler exageSamplers[]

#define GetTexture2D(index) exage2DTextures[index]
#define GetTextureCube(index) exageCubeTextures[index]
#define GetTexture3D(index) exage3DTextures[index]

#define GetSampler(index) exageSamplers[index]

#define SampleBindless2DTexture(samplerIndex, index, uv) texture(sampler2D(exage2DTextures[index], exageSamplers[samplerIndex]), uv)
#define SampleBindlessCubeTexture(samplerIndex, index, uv) texture(samplerCube(exageCubeTextures[index], exageSamplers[samplerIndex]), uv)
#define SampleBindless3DTexture(samplerIndex, index, uv) texture(sampler3D(exage3DTextures[index], exageSamplers[samplerIndex]), uv)
#define SampleBindless2DArrayTexture(samplerIndex, index, uv) texture(sampler2DArray(exage2DArrayTextures[index], exageSamplers[samplerIndex]), uv)

#define DEFINE_BINDLESS_STORAGE_BUFFER(Access, StructName) \
    layout (std430, binding = BINDLESS_STORAGE_BUFFER_BINDING) Access buffer EXAGE##StructName \
        { StructName data; } \
    EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName)[]

#define DEFINE_BINDLESS_STORAGE_BUFFER_WITH_MEMBERS(Access, StructName, members) \
    layout (std430, binding = BINDLESS_STORAGE_BUFFER_BINDING) Access buffer StructName \
        members \
    EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName)[]

#define GetBindlessStorageBuffer(StructName, index) EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName)[index].data
#define GetBindlessStorageBufferWithMembers(StructName, index) EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName)[index]

#endif // BINDLESS_SHADER_H