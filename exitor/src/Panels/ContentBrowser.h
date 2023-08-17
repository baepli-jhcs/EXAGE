#pragma once

#include <filesystem>

#include "Dialogs/FileDialog.h"
#include "exage/Projects/Project.h"
#include "exage/utils/classes.h"

namespace exitor
{

    struct ContentBrowserCallbacks
    {
        ContentBrowserCallbacks() noexcept = default;
        virtual ~ContentBrowserCallbacks() = default;

        EXAGE_DEFAULT_COPY(ContentBrowserCallbacks);
        EXAGE_DEFAULT_MOVE(ContentBrowserCallbacks);

        virtual void recognizeMesh(const std::string& path) noexcept = 0;
        virtual void recognizeMaterial(const std::string& path) noexcept = 0;
        virtual void recognizeTexture(const std::string& path) noexcept = 0;

        virtual void recognizeLevel(const std::string& path) noexcept = 0;

        virtual void onMeshSelection(const std::string& path) noexcept = 0;
        virtual void onMaterialSelection(const std::string& path) noexcept = 0;
        virtual void onTextureSelection(const std::string& path) noexcept = 0;

        virtual void onLevelSelection(const std::string& path) noexcept = 0;
    };

    class ContentBrowser
    {
      public:
        ContentBrowser() noexcept = default;
        ~ContentBrowser() = default;

        explicit ContentBrowser(ContentBrowserCallbacks& callbacks) noexcept
            : _callbacks(&callbacks)
        {
        }

        EXAGE_DELETE_COPY(ContentBrowser);
        EXAGE_DEFAULT_MOVE(ContentBrowser);

        void setCallbacks(ContentBrowserCallbacks& callbacks) noexcept { _callbacks = &callbacks; }

        void render(const std::filesystem::path& baseDirectory,
                    const exage::Projects::Project& project) noexcept;

      private:
        void eachDirectoryEntry(const std::filesystem::directory_entry& entry,
                                const std::filesystem::path& baseDirectory,
                                const exage::Projects::Project& project) noexcept;
        void assetRecognitionResults(const std::filesystem::path& baseDirectory) noexcept;

        FileDialogAsync _fileDialog;

        ContentBrowserCallbacks* _callbacks = nullptr;

        std::string _currentPath;

        std::string _selectedFile;
    };
}  // namespace exitor