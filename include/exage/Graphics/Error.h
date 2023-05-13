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
            std::filesystem::path path;
        };

        struct FileNotReadable
        {
            std::filesystem::path path;
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
