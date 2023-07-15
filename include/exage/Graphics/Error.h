#pragma once

#include <filesystem>
#include <stdexcept>
#include <utility>
#include <variant>

#include "exage/Core/Core.h"
#include "exage/utils/classes.h"

namespace exage::Graphics
{
    namespace Errors
    {
        struct SwapchainOutOfDate
        {
        };

        struct UnsupportedAPI
        {
        };

        struct FileNotFound
        {
            std::string path;
        };

        struct FileNotReadable
        {
            std::string path;
        };

        struct ShaderCompilationFailed
        {
            std::string message;
        };
    }  // namespace Errors

    using Error = std::variant<Errors::SwapchainOutOfDate,
                               Errors::UnsupportedAPI,
                               Errors::FileNotFound,
                               Errors::FileNotReadable,
                               Errors::ShaderCompilationFailed>;
}  // namespace exage::Graphics
