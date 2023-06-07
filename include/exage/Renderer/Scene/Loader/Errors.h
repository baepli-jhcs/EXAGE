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

    using AssetError = std::variant<FileNotFoundError, FileFormatError, DirectoryError, SaveError>;
}  // namespace exage::Renderer
