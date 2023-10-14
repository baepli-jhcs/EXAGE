#pragma once

#include "exage/System/Event.h"

namespace exage::System
{
    void setClipboard(WindowAPI api, std::string_view text) noexcept;
    [[nodiscard]] auto getClipboard(WindowAPI api) noexcept -> std::string;
}  // namespace exage::System