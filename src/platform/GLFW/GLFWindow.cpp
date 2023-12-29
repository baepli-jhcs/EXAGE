#ifndef GLFW_INCLUDE_NONE
#    define GLFW_INCLUDE_NONE
#    include <mutex>
#    include <queue>
#    include <unordered_set>

#    include <stdint.h>

#    include "exage/Input/KeyCode.h"
#    include "exage/System/Event.h"
#    include "exage/System/Window.h"
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

#include <bitset>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "exage/platform/GLFW/GLFWWindow.h"

namespace exage::System
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
        std::unordered_map<uint32_t, GLFWWindow*> windows;  // NOLINT
        std::unordered_map<GLFWwindow*, GLFWWindow*> glfwWindows;  // NOLINT

        auto nextID() noexcept -> uint32_t
        {
            return idCounter++;
        }

        auto getWindow(uint32_t id) noexcept -> GLFWWindow*
        {
            auto it = windows.find(id);

            if (it != windows.end())
            {
                return it->second;
            }

            return nullptr;
        }

        auto getWindow(GLFWwindow* glfwWindow) noexcept -> GLFWWindow*
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

        void windowContentScaleCallback(GLFWwindow* window, float xscale, float yscale) noexcept
        {
            auto* win = getWindow(window);

            if (win == nullptr)
            {
                return;
            }

            Events::WindowScaleChanged event {};
            event.scale = xscale;

            pushEvent({win->getID(), event});
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
                    event.modifiers = win->getModifiers();
                    event.button = toMouseButton(button);
                    pushEvent({win->getID(), event});
                    break;
                }
                case GLFW_RELEASE:
                {
                    Events::MouseButtonReleased event {};
                    event.modifiers = win->getModifiers();
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
            event.modifiers = win->getModifiers();
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
            event.modifiers = win->getModifiers();
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
                    event.modifiers = win->getModifiers();
                    event.key = toKeyCode(key);
                    pushEvent({win->getID(), event});
                    break;
                }
                case GLFW_RELEASE:
                {
                    Events::KeyReleased event {};
                    event.modifiers = win->getModifiers();
                    event.key = toKeyCode(key);
                    pushEvent({win->getID(), event});
                    break;
                }
                case GLFW_REPEAT:
                {
                    Events::KeyRepeated event {};
                    event.modifiers = win->getModifiers();
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

        std::bitset<GLFW_JOYSTICK_LAST + 1> connectedGamepads;  // NOLINT

        void joystickCallback(int jid, int connected) noexcept
        {
            if (connected == GLFW_TRUE)
            {
                if (glfwJoystickIsGamepad(jid) == 0)
                {
                    return;
                }
                connectedGamepads.set(jid);
                Events::GamepadConnected event {};
                pushEvent({static_cast<uint32_t>(jid), event});
            }
            else
            {
                if (!connectedGamepads.test(jid))
                {
                    return;
                }
                connectedGamepads.reset(jid);
                Events::GamepadDisconnected event {};
                pushEvent({static_cast<uint32_t>(jid), event});
            }
        }

        void registerWindow(uint32_t id, GLFWWindow* window, GLFWwindow* glfwWindow) noexcept
        {
            windows[id] = window;
            glfwWindows[glfwWindow] = window;

            glfwSetWindowPosCallback(glfwWindow, windowPositionCallback);
            glfwSetWindowSizeCallback(glfwWindow, windowSizeCallback);
            glfwSetWindowCloseCallback(glfwWindow, windowCloseCallback);
            glfwSetWindowFocusCallback(glfwWindow, windowFocusCallback);
            glfwSetWindowIconifyCallback(glfwWindow, windowIconifyCallback);
            glfwSetWindowMaximizeCallback(glfwWindow, windowMaximizeCallback);
            glfwSetWindowContentScaleCallback(glfwWindow, windowContentScaleCallback);
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

    void GLFWWindow::init() noexcept
    {
        glfwInit();
        glfwSetMonitorCallback(monitorCallback);
        glfwSetJoystickCallback(joystickCallback);

        // Check for connected gamepads
        for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++)
        {
            if (glfwJoystickIsGamepad(i) != 0)
            {
                connectedGamepads.set(i);
            }
        }
    }

    GLFWWindow::GLFWWindow(const WindowInfo& info) noexcept
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

    GLFWWindow::~GLFWWindow() noexcept
    {
        glfwDestroyWindow(_window);

        unregisterWindow(_id, _window);
    }

    void GLFWWindow::close() noexcept
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    auto GLFWWindow::getExtent() const noexcept -> glm::uvec2
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(_window, &width, &height);

        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    auto GLFWWindow::getPosition() const noexcept -> glm::ivec2
    {
        int xpos = 0;
        int ypos = 0;
        glfwGetWindowPos(_window, &xpos, &ypos);

        return {xpos, ypos};
    }

    auto GLFWWindow::isHidden() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_VISIBLE) == GLFW_FALSE;
    }

    auto GLFWWindow::isResizable() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_RESIZABLE) == GLFW_TRUE;
    }

    void GLFWWindow::setHidden(bool hidden) noexcept
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

    void GLFWWindow::setResizable(bool resizable) noexcept
    {
        glfwSetWindowAttrib(_window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
    }

    auto GLFWWindow::getModifiers() noexcept -> Modifiers
    {
        Modifiers modifiers {};
        if (glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eLeftShift;
        }

        if (glfwGetKey(_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eRightShift;
        }

        if (glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eLeftControl;
        }

        if (glfwGetKey(_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eRightControl;
        }

        if (glfwGetKey(_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eLeftAlt;
        }

        if (glfwGetKey(_window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eRightAlt;
        }

        if (glfwGetKey(_window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eLeftSuper;
        }

        if (glfwGetKey(_window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eRightSuper;
        }

        if (glfwGetKey(_window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eCapsLock;
        }

        if (glfwGetKey(_window, GLFW_KEY_NUM_LOCK) == GLFW_PRESS)
        {
            modifiers |= ModifierFlags::eNumLock;
        }

        return modifiers;
    }

    auto GLFWWindow::exclusiveMonitor() const noexcept -> GLFWmonitor*
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

    auto GLFWWindow::getID() const noexcept -> uint32_t
    {
        return _id;
    }

    auto GLFWWindow::getNativeHandle() const noexcept -> void*
    {
#ifdef EXAGE_WINDOWS
        return glfwGetWin32Window(_window);
#endif
#ifdef EXAGE_MACOS
        return glfwGetCocoaWindow(_window);
#endif
#ifdef EXAGE_LINUX

        return reinterpret_cast<void*>(glfwGetX11Window(_window));
#endif
    }

    void GLFWWindow::resize(glm::uvec2 extent) noexcept
    {
        glfwSetWindowSize(_window, static_cast<int>(extent.x), static_cast<int>(extent.y));
    }

    void GLFWWindow::setFullScreen(bool fullScreen) noexcept
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

    void GLFWWindow::setWindowBordered(bool bordered) noexcept
    {
        _windowBordered = bordered;

        glfwSetWindowAttrib(_window, GLFW_DECORATED, _windowBordered ? GLFW_TRUE : GLFW_FALSE);
    }

    void GLFWWindow::setExclusiveRefreshRate(uint32_t refreshRate) noexcept
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

    void GLFWWindow::setExclusiveMonitor(Monitor monitor) noexcept
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

    auto GLFWWindow::shouldClose() const noexcept -> bool
    {
        return glfwWindowShouldClose(_window) != 0;
    }

    auto GLFWWindow::isIconified() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    namespace
    {
        // For each connected gamepad, store the state of each button
        std::array<std::bitset<GLFW_GAMEPAD_BUTTON_LAST + 1>, GLFW_JOYSTICK_LAST + 1>
            gamepadButtons;  // NOLINT

        auto toGamepadButton(int button) -> GamepadButton
        {
            switch (button)
            {
                case GLFW_GAMEPAD_BUTTON_A:
                    return GamepadButton::eA;
                case GLFW_GAMEPAD_BUTTON_B:
                    return GamepadButton::eB;
                case GLFW_GAMEPAD_BUTTON_X:
                    return GamepadButton::eX;
                case GLFW_GAMEPAD_BUTTON_Y:
                    return GamepadButton::eY;
                case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
                    return GamepadButton::eLeftBumper;
                case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
                    return GamepadButton::eRightBumper;
                case GLFW_GAMEPAD_BUTTON_BACK:
                    return GamepadButton::eBack;
                case GLFW_GAMEPAD_BUTTON_START:
                    return GamepadButton::eStart;
                case GLFW_GAMEPAD_BUTTON_GUIDE:
                    return GamepadButton::eGuide;
                case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
                    return GamepadButton::eLeftThumb;
                case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
                    return GamepadButton::eRightThumb;
                case GLFW_GAMEPAD_BUTTON_DPAD_UP:
                    return GamepadButton::eDPadUp;
                case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
                    return GamepadButton::eDPadRight;
                case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
                    return GamepadButton::eDPadDown;
                case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
                    return GamepadButton::eDPadLeft;
                default:
                    break;
            }

            return GamepadButton::eA;  // Should never happen
        }

        auto toGamepadAxis(int axis) -> GamepadAxis
        {
            switch (axis)
            {
                case GLFW_GAMEPAD_AXIS_LEFT_X:
                    return GamepadAxis::eLeftX;
                case GLFW_GAMEPAD_AXIS_LEFT_Y:
                    return GamepadAxis::eLeftY;
                case GLFW_GAMEPAD_AXIS_RIGHT_X:
                    return GamepadAxis::eRightX;
                case GLFW_GAMEPAD_AXIS_RIGHT_Y:
                    return GamepadAxis::eRightY;
                case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
                    return GamepadAxis::eLeftTrigger;
                case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
                    return GamepadAxis::eRightTrigger;
                default:
                    break;
            }

            return GamepadAxis::eLeftX;  // Should never happen
        }
    }  // namespace

    void GLFWWindow::pollEvents() noexcept
    {
        glfwPollEvents();

        // Check for gamepad button events
        for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++)
        {
            if (!connectedGamepads.test(i))
            {
                continue;
            }

            GLFWgamepadstate state;
            if (glfwGetGamepadState(i, &state) == GLFW_FALSE)
            {
                continue;
            }

            for (int j = GLFW_GAMEPAD_BUTTON_A; j <= GLFW_GAMEPAD_BUTTON_LAST; j++)
            {
                if (state.buttons[j] == GLFW_PRESS)
                {
                    if (!gamepadButtons[i].test(j))
                    {
                        Events::GamepadButtonPressed event {};
                        event.button = toGamepadButton(j);

                        pushEvent({static_cast<uint32_t>(i), event});
                    }
                    gamepadButtons[i].set(j);
                }
                else
                {
                    if (gamepadButtons[i].test(j))
                    {
                        Events::GamepadButtonReleased event {};
                        event.button = toGamepadButton(j);

                        pushEvent({static_cast<uint32_t>(i), event});
                    }
                    gamepadButtons[i].reset(j);
                }
            }
        }
    }

    void GLFWWindow::waitEvents() noexcept
    {
        glfwWaitEvents();
    }

    auto GLFWWindow::getMonitorCount() noexcept -> uint32_t
    {
        int count = 0;
        glfwGetMonitors(&count);
        return count;
    }

    auto GLFWWindow::getMonitor(uint32_t index) noexcept -> Monitor
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

    auto GLFWWindow::getMonitors() noexcept -> std::vector<Monitor>
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

    auto GLFWWindow::getWindowByID(uint32_t id) noexcept -> GLFWWindow*
    {
        return getWindow(id);
    }

    auto GLFWWindow::getWindowMap() noexcept -> std::unordered_map<GLFWwindow*, GLFWWindow*>
    {
        return glfwWindows;
    }

    auto GLFWWindow::nextEvent() noexcept -> std::optional<Event>
    {
        if (events.empty())
        {
            return std::nullopt;
        }

        Event event = events.front();
        events.pop();

        return event;
    }

}  // namespace exage::System
