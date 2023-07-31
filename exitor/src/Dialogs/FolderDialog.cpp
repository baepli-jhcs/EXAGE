#include "FolderDialog.h"

#include <tinyfiledialogs.h>

#include "exage/Core/Debug.h"

namespace exitor
{
    void FolderDialogAsync::open(std::string_view title, std::string_view defaultPath) noexcept
    {
        exage::debugAssume(_ready.load(), "FolderDialogAsync is not ready");

        _ready = false;

        // allocate memory for title and defaultPath
        _title = std::string(title);
        _defaultPath = std::string(defaultPath);

        _thread = std::thread(
            [this]
            {
                const char* title = _title->c_str();
                if (_title->empty())
                {
                    title = nullptr;
                }

                const char* defaultPath = _defaultPath->c_str();
                if (_defaultPath->empty())
                {
                    defaultPath = nullptr;
                }

                char* result = tinyfd_selectFolderDialog(title, defaultPath);
                if (result != nullptr)
                {
                    _result = result;
                }
                _ready = true;
            });

        _thread.detach();
    }
}  // namespace exitor