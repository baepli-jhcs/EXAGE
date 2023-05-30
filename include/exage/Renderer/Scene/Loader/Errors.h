#pragma once

#include <string>
#include <variant>

namespace exage::Renderer
{
    struct FileNotFoundError
    {
    };

    struct FileFormatError
    {
    };

    using AssetImportError = std::variant<FileNotFoundError, FileFormatError>;
    using TextureImportError = std::variant<FileNotFoundError, FileFormatError>;
    using AssetLoadError = std::variant<FileNotFoundError, FileFormatError>;

    struct DirectoryError
    {
    };

    struct SaveError
    {
    };
}  // namespace exage::Renderer
