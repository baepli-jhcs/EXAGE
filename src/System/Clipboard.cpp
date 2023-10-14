#include "exage/System/Clipboard.h"

#include "exage/platform/GLFW/GLFWClipboard.h"

namespace exage::System
{
    void setClipboard(API api, std::string_view text) noexcept
    {
        switch (api)
        {
            case API::eGLFW:
                setClipboardGLFW(text);
                break;
            default:
                break;
        }
    }

    auto getClipboard(API api) noexcept -> std::string
    {
        switch (api)
        {
            case API::eGLFW:
                return getClipboardGLFW();
            default:
                return "";
        }
    }
}  // namespace exage::System