#pragma once

#include <variant>

#include "exage/Core/Core.h"

namespace exage::Graphics
{
    enum class GraphicsError : uint32_t
    {
        eUnsupportedAPI,
        eSwapchainOutOfDate,
    };

    enum class FileError : uint32_t
    {
        eFileNotFound,
        eFileNotReadable,
    };

    using Error = std::variant<GraphicsError, FileError>;
}  // namespace exage::Graphics
