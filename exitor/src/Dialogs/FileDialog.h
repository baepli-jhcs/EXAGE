#pragma once

#include <optional>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "exage/utils/classes.h"

namespace exitor
{
    class FileDialogAsync
    {
      public:
        FileDialogAsync() noexcept = default;
        ~FileDialogAsync() = default;

        EXAGE_DELETE_COPY(FileDialogAsync);

        FileDialogAsync(FileDialogAsync&& old) noexcept
            : _thread(std::move(old._thread))
            , _result(std::move(old._result))
            , _ready(old._ready.load())
        {
        }
        auto operator=(FileDialogAsync&& other) noexcept -> FileDialogAsync&
        {
            _thread = std::move(other._thread);
            _result = std::move(other._result);
            _ready = other._ready.load();
            return *this;
        }

        void open(std::string_view title,
                  std::string_view defaultPath,
                  std::span<std::string_view> filters,
                  std::string_view filterDescription) noexcept;
        [[nodiscard]] auto isReady() const noexcept -> bool { return _ready.load(); }

        [[nodiscard]] auto getResult() const noexcept -> std::string { return _result; }

        void clear() noexcept
        {
            _result.clear();
            _ready = false;
        }

      private:
        std::string _title;
        std::string _defaultPath;
        std::vector<std::string> _filters;
        std::vector<const char*> _filterPtrs;
        std::string _filterDescription;

        std::thread _thread;
        std::string _result;
        std::atomic<bool> _ready {false};
        std::atomic<bool> _running {false};
    };
}  // namespace exitor