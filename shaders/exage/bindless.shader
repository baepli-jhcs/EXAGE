#extension GL_EXT_nonuniform_qualifier : enable

#define BINDLESS_SAMPLED_TEXTURE_BINDING 0
#define BINDLESS_STORAGE_BUFFER_BINDING 1
#define BINDLESS_STORAGE_TEXTURE_BINDING 2

#define EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName) uExage##StructName

#define DEFINE_BINDLESS_SAMPLED_TEXTURES() \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform sampler2D exage2DTextures[]; \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform samplerCube exageCubeTextures[]; \
    layout (binding = BINDLESS_SAMPLED_TEXTURE_BINDING) uniform sampler3D exage3DTextures[]

#define SampleBindless2DTexture(index, uv) texture(exage2DTextures[index], uv)
#define SampleBindlessCubeTexture(index, uv) texture(exageCubeTextures[index], uv)
#define SampleBindless3DTexture(index, uv) texture(exage3DTextures[index], uv)

#define DEFINE_BINDLESS_STORAGE_BUFFER(Access, StructName) \
    layout (std430, binding = BINDLESS_STORAGE_BUFFER_BINDING) Access buffer EXAGE##StructName \
        { StructName data; } \
    EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName)[]

#define GetBindlessStorageBuffer(StructName, index) EXAGE_GET_BINDLESS_VARIABLE_NAME(StructName)[index].data