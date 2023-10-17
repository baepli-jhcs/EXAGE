#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Sampler.h"

namespace exage::Renderer
{
    enum class CascadeLevels : uint8_t
    {
        e1 = 1,
        e2 = 2,
        e3 = 3,
        e4 = 4,
        e5 = 5,
    };

    enum class ShadowResolution : uint32_t
    {
        e256 = 256,
        e512 = 512,
        e1024 = 1024,
        e2048 = 2048,
        e4096 = 4096,
    };

    struct RenderQualitySettings
    {
        Graphics::Sampler::Anisotropy anisotropy = Graphics::Sampler::Anisotropy::e1;
        CascadeLevels cascadeLevels = CascadeLevels::e4;
        ShadowResolution shadowResolution = ShadowResolution::e1024;
    };
}  // namespace exage::Renderer