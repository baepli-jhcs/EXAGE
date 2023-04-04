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

    namespace Errors
    {
        struct ShaderCompileFailed
        {
            std::string message;
        };
    }

    using Error = std::variant<GraphicsError, FileError, Errors::ShaderCompileFailed>;
}  // namespace exage::Graphics
