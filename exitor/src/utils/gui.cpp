#include "gui.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace exitor
{
    auto getCurrentImGuiDPI() noexcept -> float
    {
        // TODO: Currently using ImGui internals, find a better way to do this
        ImGuiViewport* viewport = ImGui::GetCurrentContext()->CurrentViewport;
        if (viewport == nullptr)
        {
            return 1.0F;
        }

        return viewport->DpiScale;
    }
}  // namespace exitor