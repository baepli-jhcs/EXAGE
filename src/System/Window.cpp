#include "exage/System/Window.h"

#include "exage/platform/GLFW/GLFWWindow.h"

namespace exage::System
{
    auto Window::create(const WindowInfo& info, API api) noexcept
        -> tl::expected<std::unique_ptr<Window>, WindowError>
    {
        switch (api)
        {
            case API::eGLFW:
                return std::make_unique<GLFWWindow>(info);
            case API::eSDL:
            default:
                return tl::make_unexpected(WindowError::eInvalidAPI);
        }
    }

    void pollEvents(API api) noexcept
    {
        switch (api)
        {
            case API::eGLFW:
                GLFWWindow::pollEvents();
                break;
            case API::eSDL:
            default:
                break;
        }
    }

    void waitEvents(API api) noexcept
    {
        switch (api)
        {
            case API::eGLFW:
                GLFWWindow::waitEvents();
                break;
            case API::eSDL:
            default:
                break;
        }
    }

    auto nextEvent(API api) noexcept -> std::optional<Event>
    {
        switch (api)
        {
            case API::eGLFW:
                return GLFWWindow::nextEvent();
            case API::eSDL:
            default:
                return {};
        }
    }

    auto getMonitorCount(API api) noexcept -> uint32_t
    {
        switch (api)
        {
            case API::eGLFW:
                return GLFWWindow::getMonitorCount();
            case API::eSDL:
            default:
                return 0;
        }
    }

    auto getMonitor(uint32_t index, API api) noexcept -> Monitor
    {
        switch (api)
        {
            case API::eGLFW:
                return GLFWWindow::getMonitor(index);
            case API::eSDL:
            default:
                return {};
        }
    }

    auto getMonitors(API api) noexcept -> std::vector<Monitor>
    {
        switch (api)
        {
            case API::eGLFW:
                return GLFWWindow::getMonitors();
            case API::eSDL:
            default:
                return {};
        }
    }
}  // namespace exage::System
