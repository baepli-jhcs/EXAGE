#pragma once

#include <string>
#include <variant>

namespace exage
{
    namespace Errors
    {
        struct FileNotFound
        {
        };

        struct FileFormat
        {
        };

        struct DirectoryMissing
        {
        };

        struct SerializationFailed
        {
        };

        struct DeserializationFailed
        {
        };
    }  // namespace Errors

    using Error = std::variant<Errors::FileNotFound,
                               Errors::FileFormat,
                               Errors::DirectoryMissing,
                               Errors::SerializationFailed,
                               Errors::DeserializationFailed>;
}  // namespace exage
