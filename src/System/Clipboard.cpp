#include "exage/System/Clipboard.h"

#include "exage/platform/GLFW/GLFWClipboard.h"

namespace exage::System
{
    void setClipboard(WindowAPI api, std::string_view text) noexcept
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                setClipboardGLFW(text);
                break;
            default:
                break;
        }
    }

    auto getClipboard(WindowAPI api) noexcept -> std::string
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return getClipboardGLFW();
            default:
                return "";
        }
    }
}  // namespace exage::System