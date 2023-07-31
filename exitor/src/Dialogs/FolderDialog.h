#pragma once

#include <optional>
#include <string>
#include <thread>

#include "exage/utils/classes.h"

namespace exitor
{
    class FolderDialogAsync
    {
      public:
        FolderDialogAsync() noexcept = default;
        ~FolderDialogAsync() = default;

        EXAGE_DELETE_COPY(FolderDialogAsync);

        FolderDialogAsync(FolderDialogAsync&& old) noexcept
            : _thread(std::move(old._thread))
            , _result(std::move(old._result))
            , _ready(old._ready.load())
        {
        }
        auto operator=(FolderDialogAsync&& other) noexcept -> FolderDialogAsync&
        {
            _thread = std::move(other._thread);
            _result = std::move(other._result);
            _ready = other._ready.load();
            return *this;
        }

        void open(std::string_view title, std::string_view defaultPath) noexcept;
        [[nodiscard]] auto isReady() const noexcept -> bool { return _ready.load(); }

        [[nodiscard]] auto getResult() const noexcept -> std::string { return _result; }

      private:
        std::optional<std::string> _title;
        std::optional<std::string> _defaultPath;

        std::thread _thread;
        std::string _result;
        std::atomic<bool> _ready {true};
    };
}  // namespace exitor