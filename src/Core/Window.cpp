#include "Core/Window.h"

#include "GLFW/GLFWindow.h"

namespace exage
{

    auto Window::create(const WindowInfo& info, WindowAPI api) -> Window*
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return new GLFWindow(info);
            default:
                return nullptr;
        }
    }
}  // namespace exage