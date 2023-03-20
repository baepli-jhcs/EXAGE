#include "Core/Window.h"

#include "GLFW/GLFWindow.h"

namespace exage
{
    auto Window::create(const WindowInfo& info, WindowAPI api) noexcept
        -> tl::expected<std::unique_ptr<Window>, WindowError>
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return std::make_unique<GLFWindow>(info);
            case WindowAPI::eSDL:
            default:
                return tl::make_unexpected(WindowError::eInvalidAPI);
        }
    }

    auto getMonitorCount(WindowAPI api) noexcept -> uint32_t
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return GLFWindow::getMonitorCount();
            case WindowAPI::eSDL:
            default:
                return 0;
        }
    }

    auto getMonitor(uint32_t index, WindowAPI api) noexcept -> Monitor
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return GLFWindow::getMonitor(index);
            case WindowAPI::eSDL:
            default:
                return {};
        }
    }

    auto getMonitors(WindowAPI api) noexcept -> std::vector<Monitor>
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return GLFWindow::getMonitors();
            case WindowAPI::eSDL:
            default:
                return {};
        }
    }
}  // namespace exage
