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
            -> std::optional<ModelImportDetails>;

      private:
        std::string _modelPath;
        std::string _texturePath;

        std::string _currentModelOutputPath;
        std::string _outputDirectory;
        std::string _outputPath;

        bool _importIntoScene = false;

        FileDialogAsync _dialog;
    };
}  // namespace exitor