#include "exage/System/Window.h"

#include "exage/platform/GLFW/GLFWindow.h"

namespace exage::System
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

    void pollEvents(WindowAPI api) noexcept
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                GLFWindow::pollEvents();
                break;
            case WindowAPI::eSDL:
            default:
                break;
        }
    }

    void waitEvents(WindowAPI api) noexcept
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                GLFWindow::waitEvents();
                break;
            case WindowAPI::eSDL:
            default:
                break;
        }
    }

    auto nextEvent(WindowAPI api) noexcept -> std::optional<Event>
    {
        switch (api)
        {
            case WindowAPI::eGLFW:
                return GLFWindow::nextEvent();
            case WindowAPI::eSDL:
            default:
                return {};
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
}  // namespace exage::System
