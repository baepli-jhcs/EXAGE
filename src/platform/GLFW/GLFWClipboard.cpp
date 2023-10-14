#include "exage/platform/GLFW/GLFWClipboard.h"

#include <GLFW/glfw3.h>

namespace exage::System
{
    void setClipboardGLFW(std::string_view text) noexcept
    {
        const std::string str(text);
        glfwSetClipboardString(nullptr, str.c_str());
    }

    auto getClipboardGLFW() noexcept -> std::string
    {
        return glfwGetClipboardString(nullptr);
    }
}  // namespace exage::System