#pragma once

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Graphics/BindlessResources.h"
#include "exage/Graphics/Context.h"
#include "exage/utils/classes.h"

namespace exage::Graphics
{
    struct SamplerCreateInfo;

    class Sampler
    {
      public:
        virtual ~Sampler() = default;
        EXAGE_DELETE_COPY(Sampler);
        EXAGE_DEFAULT_MOVE(Sampler);

        enum class Anisotropy : uint32_t
        {
            eDisabled = 0,
            e1 = 1,
            e2 = 2,
            e4 = 4,
            e8 = 8,
            e16 = 16
        };

        enum class Filter : uint32_t
        {
            eNearest = 0,
            eLinear = 1
        };

        enum class MipmapMode : uint32_t
        {
            eNearest = 0,
            eLinear = 1
        };

        [[nodiscard]] auto getAnisotropy() const noexcept -> Anisotropy { return _anisotropy; }
        [[nodiscard]] auto getFilter() const noexcept -> Filter { return _filter; }
        [[nodiscard]] auto getMipmapMode() const noexcept -> MipmapMode { return _mipmapMode; }
        [[nodiscard]] auto getLodBias() const noexcept -> float { return _lodBias; }

        [[nodiscard]] auto getBindlessID() const noexcept -> SamplerID { return _id; }

        [[nodiscard]] auto getSamplerCreateInfo() const noexcept -> SamplerCreateInfo;

        EXAGE_BASE_API(API, Sampler);

      protected:
        Anisotropy _anisotropy;
        Filter _filter;
        MipmapMode _mipmapMode;
        float _lodBias;

        SamplerID _id {};

        Sampler(Anisotropy anisotropy, Filter filter, MipmapMode mipmapMode, float lodBias) noexcept
            : _anisotropy(anisotropy)
            , _filter(filter)
            , _mipmapMode(mipmapMode)
            , _lodBias(lodBias)
        {
        }
    };

    struct SamplerCreateInfo
    {
        Sampler::Anisotropy anisotropy = Sampler::Anisotropy::eDisabled;
        Sampler::Filter filter = Sampler::Filter::eLinear;
        Sampler::MipmapMode mipmapMode = Sampler::MipmapMode::eLinear;
        float lodBias = 0.0F;
    };
}  // namespace exage::Graphics
