#ifndef GLFW_INCLUDE_NONE
#    define GLFW_INCLUDE_NONE
#    include <mutex>
#    include <queue>
#    include <unordered_set>

#    include <stdint.h>

#    include "exage/Core/Event.h"
#    include "exage/Core/Window.h"
#    include "exage/Input/KeyCode.h"
#endif

#ifdef EXAGE_WINDOWS
#    define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifdef EXAGE_LINUX
#    define GLFW_EXPOSE_NATIVE_X11
#endif
#ifdef EXAGE_MACOS
#    define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "exage/platform/GLFW/GLFWindow.h"

namespace exage
{
    namespace
    {
        auto toKeyCode(int key) -> KeyCode
        {
            KeyCode keyCode {};
            if (key == GLFW_KEY_UNKNOWN || key > GLFW_KEY_LAST)
            {
                keyCode.code = KeyCode::eUnknown;
            }

            keyCode.code = static_cast<KeyCode::Codes>(key);
            return keyCode;
        }

        auto toMouseButton(int button) -> MouseButton
        {
            return {static_cast<MouseButton::Codes>(button)};
        }

        uint32_t idCounter = 0;  // NOLINT
        std::unordered_map<uint32_t, GLFWindow*> windows;  // NOLINT
        std::unordered_map<GLFWwindow*, GLFWindow*> glfwWindows;  // NOLINT

        auto nextID() noexcept -> uint32_t
        {
            return idCounter++;
        }

        auto getWindow(uint32_t id) noexcept -> GLFWindow*
        {
            auto it = windows.find(id);

            if (it != windows.end())
            {
                return it->second;
            }

            return nullptr;
        }

        auto getWindow(GLFWwindow* glfwWindow) noexcept -> GLFWindow*
        {
            auto it = glfwWindows.find(glfwWindow);

            if (it != glfwWindows.end())
            {
                return it->second;
            }

            return nullptr;
        }

        std::queue<Event> events;  // NOLINT

        void pushEvent(Event event) noexcept
        {
            events.push(std::move(event));
        }

        void windowPositionCallback(GLFWwindow* window, int xpos, int ypos) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::WindowMoved event {};
            event.position = {xpos, ypos};

            pushEvent({win->getID(), event});
        }

        void windowSizeCallback(GLFWwindow* window, int width, int height) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::WindowResized event {};
            event.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            pushEvent({win->getID(), event});
        }

        void windowCloseCallback(GLFWwindow* window) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::WindowClosed event {};

            pushEvent({win->getID(), event});
        }

        void windowFocusCallback(GLFWwindow* window, int focused) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            if (focused == GLFW_TRUE)
            {
                Events::WindowFocused event {};
                pushEvent({win->getID(), event});
            }
            else
            {
                Events::WindowLostFocus event {};
                pushEvent({win->getID(), event});
            }
        }

        void windowIconifyCallback(GLFWwindow* window, int iconified) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            if (iconified == GLFW_TRUE)
            {
                Events::WindowIconified event {};
                pushEvent({win->getID(), event});
            }
            else
            {
                Events::WindowRestored event {};
                pushEvent({win->getID(), event});
            }
        }

        void windowMaximizeCallback(GLFWwindow* window, int maximized) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            if (maximized == GLFW_TRUE)
            {
                Events::WindowMaximized event {};
                pushEvent({win->getID(), event});
            }
            else
            {
                Events::WindowUnmaximized event {};
                pushEvent({win->getID(), event});
            }
        }

        void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            switch (action)
            {
                case GLFW_PRESS:
                {
                    Events::MouseButtonPressed event {};
                    event.button = toMouseButton(button);
                    pushEvent({win->getID(), event});
                    break;
                }
                case GLFW_RELEASE:
                {
                    Events::MouseButtonReleased event {};
                    event.button = toMouseButton(button);
                    pushEvent({win->getID(), event});
                    break;
                }
                default:
                    break;
            }
        }

        void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::MouseMoved event {};
            event.position = {static_cast<int>(xpos), static_cast<int>(ypos)};

            pushEvent({win->getID(), event});
        }

        void scrollCallback(GLFWwindow* window, double x, double y) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::MouseScrolled event {};
            event.offset = {static_cast<float>(x), static_cast<float>(y)};

            pushEvent({win->getID(), event});
        }

        void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            if (entered == GLFW_TRUE)
            {
                Events::MouseEntered event {};
                pushEvent({win->getID(), event});
            }
            else
            {
                Events::MouseLeft event {};
                pushEvent({win->getID(), event});
            }
        }

        void keyCallback(
            GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            switch (action)
            {
                case GLFW_PRESS:
                {
                    Events::KeyPressed event {};
                    event.key = toKeyCode(key);
                    pushEvent({win->getID(), event});
                    break;
                }
                case GLFW_RELEASE:
                {
                    Events::KeyReleased event {};
                    event.key = toKeyCode(key);
                    pushEvent({win->getID(), event});
                    break;
                }
                case GLFW_REPEAT:
                {
                    Events::KeyRepeated event {};
                    event.key = toKeyCode(key);
                    pushEvent({win->getID(), event});
                    break;
                }
                default:
                    break;
            }
        }

        void charCallback(GLFWwindow* window, unsigned int codepoint) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::CodepointInput event {};
            event.codepoint = codepoint;

            pushEvent({win->getID(), event});
        }

        void dropCallback(GLFWwindow* window, int count, const char** paths) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            for (int i = 0; i < count; i++)
            {
                Events::FileDropped event {};
                event.path = paths[i];

                pushEvent({win->getID(), event});
            }
        }

        void monitorCallback(GLFWmonitor* monitor, int connected) noexcept
        {
            // Search for the monitor in the list of all monitors
            int monitorCount = 0;
            GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

            for (int i = 0; i < monitorCount; i++)
            {
                if (monitors[i] == monitor)
                {
                    // Found the monitor
                    if (connected == GLFW_TRUE)
                    {
                        // Monitor connected
                        Events::MonitorConnected event {};
                        pushEvent({static_cast<uint32_t>(i), event});
                    }
                }
            }

            if (connected == GLFW_FALSE)
            {
                // Monitor disconnected
                Events::MonitorDisconnected event {};
                pushEvent({static_cast<uint32_t>(monitorCount), event});
            }
        }

        void joystickCallback(int jid, int connected) noexcept
        {
            if (connected == GLFW_TRUE)
            {
                // Joystick connected
                Events::JoystickConnected event {};
                pushEvent({static_cast<uint32_t>(jid), event});
            }
            else
            {
                // Joystick disconnected
                Events::JoystickDisconnected event {};
                pushEvent({static_cast<uint32_t>(jid), event});
            }
        }

        void registerWindow(uint32_t id, GLFWindow* window, GLFWwindow* glfwWindow) noexcept
        {
            windows[id] = window;
            glfwWindows[glfwWindow] = window;

            glfwSetWindowPosCallback(glfwWindow, windowPositionCallback);
            glfwSetWindowSizeCallback(glfwWindow, windowSizeCallback);
            glfwSetWindowCloseCallback(glfwWindow, windowCloseCallback);
            glfwSetWindowFocusCallback(glfwWindow, windowFocusCallback);
            glfwSetWindowIconifyCallback(glfwWindow, windowIconifyCallback);
            glfwSetWindowMaximizeCallback(glfwWindow, windowMaximizeCallback);
            glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
            glfwSetCursorPosCallback(glfwWindow, cursorPosCallback);
            glfwSetScrollCallback(glfwWindow, scrollCallback);
            glfwSetCursorEnterCallback(glfwWindow, cursorEnterCallback);
            glfwSetKeyCallback(glfwWindow, keyCallback);
            glfwSetCharCallback(glfwWindow, charCallback);
            glfwSetDropCallback(glfwWindow, dropCallback);
        }

        void unregisterWindow(uint32_t id, GLFWwindow* glfwWindow) noexcept
        {
            windows.erase(id);
            glfwWindows.erase(glfwWindow);
        }
    }  // namespace

    void GLFWindow::init() noexcept
    {
        glfwInit();
        glfwSetMonitorCallback(monitorCallback);
        glfwSetJoystickCallback(joystickCallback);
    }

    GLFWindow::GLFWindow(const WindowInfo& info) noexcept
        : _id(nextID())
        , _name(info.name)
        , _extent(info.extent)
        , _fullScreen(info.fullScreen)
        , _windowBordered(info.windowBordered)
        , _exclusiveRefreshRate(info.exclusiveRefreshRate)
        , _exclusiveMonitor(info.exclusiveMonitor)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, info.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, info.hidden ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_FOCUSED, info.focused ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, info.focusOnShow ? GLFW_TRUE : GLFW_FALSE);

        if (info.fullScreen)
        {
            glfwWindowHint(GLFW_REFRESH_RATE, static_cast<int>(_exclusiveRefreshRate));

            _window = glfwCreateWindow(static_cast<int>(_extent.x),
                                       static_cast<int>(_extent.y),
                                       _name.c_str(),
                                       exclusiveMonitor(),
                                       nullptr);
        }
        else
        {
            if (_windowBordered)
            {
                glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            }
            else
            {
                glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            }

            _window = glfwCreateWindow(static_cast<int>(_extent.x),
                                       static_cast<int>(_extent.y),
                                       _name.c_str(),
                                       nullptr,
                                       nullptr);
        }

        glfwSetWindowUserPointer(_window, this);
        registerWindow(_id, this, _window);
    }

    GLFWindow::~GLFWindow() noexcept
    {
        glfwDestroyWindow(_window);

        unregisterWindow(_id, _window);
    }

    void GLFWindow::close() noexcept
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    auto GLFWindow::getExtent() const noexcept -> glm::uvec2
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(_window, &width, &height);

        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    auto GLFWindow::getPosition() const noexcept -> glm::ivec2
    {
        int xpos = 0;
        int ypos = 0;
        glfwGetWindowPos(_window, &xpos, &ypos);

        return {xpos, ypos};
    }

    auto GLFWindow::isHidden() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_VISIBLE) == GLFW_FALSE;
    }

    auto GLFWindow::isResizable() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_RESIZABLE) == GLFW_TRUE;
    }

    void GLFWindow::setHidden(bool hidden) noexcept
    {
        if (hidden)
        {
            glfwHideWindow(_window);
        }
        else
        {
            glfwShowWindow(_window);
        }
    }

    void GLFWindow::setResizable(bool resizable) noexcept
    {
        glfwSetWindowAttrib(_window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
    }

    auto GLFWindow::exclusiveMonitor() const noexcept -> GLFWmonitor*
    {
        int monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        for (int i = 0; i < monitorCount; i++)
        {
            std::string_view const name = glfwGetMonitorName(monitors[i]);

            if (name.compare(_exclusiveMonitor.name) == 0)
            {
                return monitors[i];
            }
        }

        return nullptr;
    }

    auto GLFWindow::getID() const noexcept -> uint32_t
    {
        return _id;
    }

    auto GLFWindow::getNativeHandle() const noexcept -> void*
    {
#ifdef EXAGE_WINDOWS
        return glfwGetWin32Window(_window);
#endif
#ifdef EXAGE_MACOS
        return glfwGetCocoaWindow(_window);
#endif
#ifdef EXAGE_LINUX

        return glfwGetX11Window(_window);
#endif
    }

    void GLFWindow::resize(glm::uvec2 extent) noexcept
    {
        glfwSetWindowSize(_window, static_cast<int>(extent.x), static_cast<int>(extent.y));
    }

    void GLFWindow::setFullScreen(bool fullScreen) noexcept
    {
        _fullScreen = fullScreen;

        if (_fullScreen)
        {
            glfwSetWindowMonitor(_window,
                                 exclusiveMonitor(),
                                 0,
                                 0,
                                 static_cast<int>(_extent.x),
                                 static_cast<int>(_extent.y),
                                 static_cast<int>(_exclusiveRefreshRate));
        }
        else
        {
            int xpos = 0;
            int ypos = 0;
            glfwGetWindowPos(_window, &xpos, &ypos);

            glfwSetWindowMonitor(_window,
                                 nullptr,
                                 xpos,
                                 ypos,
                                 static_cast<int>(_extent.x),
                                 static_cast<int>(_extent.y),
                                 static_cast<int>(_exclusiveRefreshRate));
        }
    }

    void GLFWindow::setWindowBordered(bool bordered) noexcept
    {
        _windowBordered = bordered;

        glfwSetWindowAttrib(_window, GLFW_DECORATED, _windowBordered ? GLFW_TRUE : GLFW_FALSE);
    }

    void GLFWindow::setExclusiveRefreshRate(uint32_t refreshRate) noexcept
    {
        _exclusiveRefreshRate = refreshRate;

        if (_fullScreen)
        {
            glfwSetWindowMonitor(_window,
                                 exclusiveMonitor(),
                                 0,
                                 0,
                                 static_cast<int>(_extent.x),
                                 static_cast<int>(_extent.y),
                                 static_cast<int>(_exclusiveRefreshRate));
        }
    }

    void GLFWindow::setExclusiveMonitor(Monitor monitor) noexcept
    {
        _exclusiveMonitor = monitor;

        if (_fullScreen)
        {
            glfwSetWindowMonitor(_window,
                                 exclusiveMonitor(),
                                 0,
                                 0,
                                 static_cast<int>(_extent.x),
                                 static_cast<int>(_extent.y),
                                 static_cast<int>(_exclusiveRefreshRate));
        }
    }

    auto GLFWindow::shouldClose() const noexcept -> bool
    {
        return glfwWindowShouldClose(_window) != 0;
    }

    auto GLFWindow::isIconified() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    void GLFWindow::pollEvents() noexcept
    {
        glfwPollEvents();
    }

    void GLFWindow::waitEvents() noexcept
    {
        glfwWaitEvents();
    }

    auto GLFWindow::getMonitorCount() noexcept -> uint32_t
    {
        int count = 0;
        glfwGetMonitors(&count);
        return count;
    }

    auto GLFWindow::getMonitor(uint32_t index) noexcept -> Monitor
    {
        int count = static_cast<int>(getMonitorCount());
        auto countU = static_cast<uint32_t>(count);

        debugAssert(index < countU, "Monitor index out of bounds");

        GLFWmonitor* monitor = glfwGetMonitors(&count)[index];
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        Monitor mon;
        mon.name = glfwGetMonitorName(monitor);
        mon.extent = {static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height)};
        mon.refreshRate = static_cast<uint32_t>(mode->refreshRate);

        return mon;
    }

    auto GLFWindow::getMonitors() noexcept -> std::vector<Monitor>
    {
        int const count = static_cast<int>(getMonitorCount());

        std::vector<Monitor> monitors;
        monitors.reserve(count);

        for (int i = 0; i < count; i++)
        {
            monitors.push_back(getMonitor(i));
        }

        return monitors;
    }

    auto GLFWindow::getWindowByID(uint32_t id) noexcept -> GLFWindow*
    {
        return getWindow(id);
    }

    auto GLFWindow::nextEvent() noexcept -> std::optional<Event>
    {
        if (events.empty())
        {
            return std::nullopt;
        }

        Event event = events.front();
        events.pop();

        return event;
    }

}  // namespace exage
