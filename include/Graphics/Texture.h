#pragma once

#include "glm/glm.hpp"
#include "utils/classes.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT Texture
    {
      public:
        enum class Format;
        enum class Type;
        enum class Layout;

        Texture() = default;
        virtual ~Texture() = default;
        EXAGE_DELETE_COPY(Texture);
        EXAGE_DEFAULT_MOVE(Texture);

        virtual auto getFormat() const -> Format = 0;
        virtual auto getExtent() const -> glm::uvec3 = 0;
        virtual auto getType() const -> Type = 0;
        virtual auto getLayout() const -> Layout = 0;

        enum class Format
        {
            eR8,
            eR16,
            eR32,
            eRG8,
            eRG16,
            eRG32,
            eRGB8,
            eRGB16,
            eRGB32,
            eRGBA8,
            eRGBA16,
            eRGBA32,

            eDepth24Stencil8,
            eDepth32Stencil8,
        };

        enum class Type
        {
            e1D,
            e2D,
            e3D,
        };

        enum class Layout
        {
            eUndefined,
            eDepthStencilAttachment,
            eReadOnly,
            eAttachment,
            eTransferSource,
            eTransferDestination,
        };
    };
}  // namespace exage::Graphics
