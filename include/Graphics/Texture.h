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

        [[nodiscard]] virtual auto getFormat() const -> Format = 0;
        [[nodiscard]] virtual auto getExtent() const -> glm::uvec3 = 0;
        [[nodiscard]] virtual auto getType() const -> Type = 0;
        [[nodiscard]] virtual auto getLayout() const -> Layout = 0;

        enum class Format
        {
            eR8,
            eR16,
            eRG8,
            eRG16,
            eRGB8,
            eRGB16,
            eRGBA8,
            eRGBA16,

            eR16f,
            eRG16f,
            eRGB16f,
            eRGBA16f,

            eR32f,
            eRG32f,
            eRGB32f,
            eRGBA32f,

            eDepth24Stencil8,
            eDepth32Stencil8,

            eBGRA8,
            // ONLY USE FOR SWAPCHAIN
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
} // namespace exage::Graphics
