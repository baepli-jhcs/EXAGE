#pragma once

#include "Dialogs/FileDialog.h"
#include "exage/Core/Core.h"
#include "exage/Projects/Project.h"

namespace exitor
{
    constexpr static const char* IMPORT_MODEL_ID = "Import Model";
    constexpr static const char* IMPORT_TEXTURE_ID = "Import Texture";

    struct ModelImportDetails
    {
        std::string modelPath;
        std::string outputDirectory;
        bool importIntoScene = false;
        bool optimizeTexturePrecision = false;
    };

    struct TextureImportDetails
    {
        std::string texturePath;
        std::string outputPath;
        bool optimizePrecision = false;
    };

    class AssetImport
    {
      public:
        AssetImport() noexcept = default;
        ~AssetImport() = default;

        EXAGE_DELETE_COPY(AssetImport);
        EXAGE_DEFAULT_MOVE(AssetImport);

        [[nodiscard]] auto importAssetScreen(const std::filesystem::path& basePath,
                                             const exage::Projects::Project& project) noexcept
            -> std::pair<std::optional<ModelImportDetails>, std::optional<TextureImportDetails>>;

      private:
        std::string _modelPath;
        std::string _texturePath;

        std::string _currentModelOutputPath;
        std::string _outputDirectory;
        std::string _currentTextureOutputPath;
        std::string _outputTextureDirectory;
        std::string _outputTextureName;

        bool _importIntoScene = false;
        bool _optimizeTexturePrecision = false;

        bool _optimizePrecision = false;

        FileDialogAsync _dialog;
    };
}  // namespace exitor