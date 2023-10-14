#pragma once

#include "exage/System/Event.h"

namespace exage::System
{
    void setClipboard(API api, std::string_view text) noexcept;
    [[nodiscard]] auto getClipboard(API api) noexcept -> std::string;
}  // namespace exage::System