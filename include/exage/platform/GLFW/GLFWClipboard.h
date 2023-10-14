#pragma once

#include "exage/System/Clipboard.h"

namespace exage::System
{
    void setClipboardGLFW(std::string_view text) noexcept;
    [[nodiscard]] auto getClipboardGLFW() noexcept -> std::string;
}  // namespace exage::System