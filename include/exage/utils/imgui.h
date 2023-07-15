#pragma once

#include <imgui.h>

#include "exage/Core/Core.h"

namespace exage::ImGuiUtils
{
    inline void Spacing(uint32_t count = 1) noexcept
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            ImGui::Spacing();
        }
    }
}  // namespace exage::ImGuiUtils