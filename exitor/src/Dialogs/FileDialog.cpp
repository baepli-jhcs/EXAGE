#include "FileDialog.h"

#include <tinyfiledialogs.h>

#include "exage/Core/Debug.h"

namespace exitor
{
    void FileDialogAsync::open(std::string_view title,
                               std::string_view defaultPath,
                               std::span<std::string_view> filters,
                               std::string_view filterDescription) noexcept
    {
        exage::debugAssume(!_running.load(), "FileDialogAsync is already running");

        _ready = false;

        // allocate memory for title and defaultPath
        _title = std::string(title);
        _defaultPath = std::string(defaultPath);

        _filters.clear();
        _filters.reserve(filters.size());
        for (auto& filter : filters)
        {
            _filters.emplace_back(filter);
        }

        _filterDescription = std::string(filterDescription);

        _filterPtrs.clear();
        _filterPtrs.reserve(_filters.size());
        for (auto& filter : _filters)
        {
            _filterPtrs.emplace_back(filter.c_str());
        }

        _thread = std::thread(
            [this]
            {
                const char* title = _title.c_str();
                if (_title.empty())
                {
                    title = nullptr;
                }

                const char* defaultPath = _defaultPath.c_str();
                if (_defaultPath.empty())
                {
                    defaultPath = nullptr;
                }

                const char* filterDescription = _filterDescription.c_str();
                if (_filterDescription.empty())
                {
                    filterDescription = nullptr;
                }

                char* result = tinyfd_openFileDialog(title,
                                                     defaultPath,
                                                     static_cast<int>(_filters.size()),
                                                     _filterPtrs.data(),
                                                     filterDescription,
                                                     0);
                if (result != nullptr)
                {
                    _result = result;
                }
                _ready = true;
                _running = false;
            });

        _thread.detach();
    }
}  // namespace exitor