#pragma once

#include <variant>

#include "Core/Core.h"

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
