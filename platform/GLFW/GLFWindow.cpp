#ifndef GLFW_INCLUDE_NONE
#    define GLFW_INCLUDE_NONE
#    include "Core/Window.h"
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

#include "GLFWindow.h"

namespace exage
{
    namespace
    {
        KeyCode toKeyCode(int key)
        {
            switch (key)
            {
                case GLFW_KEY_SPACE:
                    return KeyCode::eSpace;
                case GLFW_KEY_APOSTROPHE:
                    return KeyCode::eApostrophe;
                case GLFW_KEY_COMMA:
                    return KeyCode::eComma;
                case GLFW_KEY_MINUS:
                    return KeyCode::eMinus;
                case GLFW_KEY_PERIOD:
                    return KeyCode::ePeriod;
                case GLFW_KEY_SLASH:
                    return KeyCode::eSlash;
                case GLFW_KEY_0:
                    return KeyCode::e0;
                case GLFW_KEY_1:
                    return KeyCode::e1;
                case GLFW_KEY_2:
                    return KeyCode::e2;
                case GLFW_KEY_3:
                    return KeyCode::e3;
                case GLFW_KEY_4:
                    return KeyCode::e4;
                case GLFW_KEY_5:
                    return KeyCode::e5;
                case GLFW_KEY_6:
                    return KeyCode::e6;
                case GLFW_KEY_7:
                    return KeyCode::e7;
                case GLFW_KEY_8:
                    return KeyCode::e8;
                case GLFW_KEY_9:
                    return KeyCode::e9;
                case GLFW_KEY_SEMICOLON:
                    return KeyCode::eSemicolon;
                case GLFW_KEY_EQUAL:
                    return KeyCode::eEqual;
                case GLFW_KEY_A:
                    return KeyCode::eA;
                case GLFW_KEY_B:
                    return KeyCode::eB;
                case GLFW_KEY_C:
                    return KeyCode::eC;
                case GLFW_KEY_D:
                    return KeyCode::eD;
                case GLFW_KEY_E:
                    return KeyCode::eE;
                case GLFW_KEY_F:
                    return KeyCode::eF;
                case GLFW_KEY_G:
                    return KeyCode::eG;
                case GLFW_KEY_H:
                    return KeyCode::eH;
                case GLFW_KEY_I:
                    return KeyCode::eI;
                case GLFW_KEY_J:
                    return KeyCode::eJ;
                case GLFW_KEY_K:
                    return KeyCode::eK;
                case GLFW_KEY_L:
                    return KeyCode::eL;
                case GLFW_KEY_M:
                    return KeyCode::eM;
                case GLFW_KEY_N:
                    return KeyCode::eN;
                case GLFW_KEY_O:
                    return KeyCode::eO;
                case GLFW_KEY_P:
                    return KeyCode::eP;
                case GLFW_KEY_Q:
                    return KeyCode::eQ;
                case GLFW_KEY_R:
                    return KeyCode::eR;
                case GLFW_KEY_S:
                    return KeyCode::eS;
                case GLFW_KEY_T:
                    return KeyCode::eT;
                case GLFW_KEY_U:
                    return KeyCode::eU;
                case GLFW_KEY_V:
                    return KeyCode::eV;
                case GLFW_KEY_W:
                    return KeyCode::eW;
                case GLFW_KEY_X:
                    return KeyCode::eX;
                case GLFW_KEY_Y:
                    return KeyCode::eY;
                case GLFW_KEY_Z:
                    return KeyCode::eZ;
                case GLFW_KEY_LEFT_BRACKET:
                    return KeyCode::eLeftBracket;
                case GLFW_KEY_BACKSLASH:
                    return KeyCode::eBackslash;
                case GLFW_KEY_RIGHT_BRACKET:
                    return KeyCode::eRightBracket;
                case GLFW_KEY_GRAVE_ACCENT:
                    return KeyCode::eGraveAccent;
                case GLFW_KEY_WORLD_1:
                    return KeyCode::eWorld1;
                case GLFW_KEY_WORLD_2:
                    return KeyCode::eWorld2;
                case GLFW_KEY_ESCAPE:
                    return KeyCode::eEscape;
                case GLFW_KEY_ENTER:
                    return KeyCode::eEnter;
                case GLFW_KEY_TAB:
                    return KeyCode::eTab;
                case GLFW_KEY_BACKSPACE:
                    return KeyCode::eBackspace;
                case GLFW_KEY_INSERT:
                    return KeyCode::eInsert;
                case GLFW_KEY_DELETE:
                    return KeyCode::eDelete;
                case GLFW_KEY_RIGHT:
                    return KeyCode::eRight;
                case GLFW_KEY_LEFT:
                    return KeyCode::eLeft;
                case GLFW_KEY_UP:
                    return KeyCode::eUp;
                case GLFW_KEY_DOWN:
                    return KeyCode::eDown;
                case GLFW_KEY_PAGE_UP:
                    return KeyCode::ePageUp;
                case GLFW_KEY_PAGE_DOWN:
                    return KeyCode::ePageDown;
                case GLFW_KEY_HOME:
                    return KeyCode::eHome;
                case GLFW_KEY_END:
                    return KeyCode::eEnd;
                case GLFW_KEY_CAPS_LOCK:
                    return KeyCode::eCapsLock;
                case GLFW_KEY_SCROLL_LOCK:
                    return KeyCode::eScrollLock;
                case GLFW_KEY_NUM_LOCK:
                    return KeyCode::eNumLock;
                case GLFW_KEY_PRINT_SCREEN:
                    return KeyCode::ePrintScreen;
                case GLFW_KEY_PAUSE:
                    return KeyCode::ePause;
                case GLFW_KEY_F1:
                    return KeyCode::eF1;
                case GLFW_KEY_F2:
                    return KeyCode::eF2;
                case GLFW_KEY_F3:
                    return KeyCode::eF3;
                case GLFW_KEY_F4:
                    return KeyCode::eF4;
                case GLFW_KEY_F5:
                    return KeyCode::eF5;
                case GLFW_KEY_F6:
                    return KeyCode::eF6;
                case GLFW_KEY_F7:
                    return KeyCode::eF7;
                case GLFW_KEY_F8:
                    return KeyCode::eF8;
                case GLFW_KEY_F9:
                    return KeyCode::eF9;
                case GLFW_KEY_F10:
                    return KeyCode::eF10;
                case GLFW_KEY_F11:
                    return KeyCode::eF11;
                case GLFW_KEY_F12:
                    return KeyCode::eF12;
                case GLFW_KEY_KP_0:
                    return KeyCode::eKP0;
                case GLFW_KEY_KP_1:
                    return KeyCode::eKP1;
                case GLFW_KEY_KP_2:
                    return KeyCode::eKP2;
                case GLFW_KEY_KP_3:
                    return KeyCode::eKP3;
                case GLFW_KEY_KP_4:
                    return KeyCode::eKP4;
                case GLFW_KEY_KP_5:
                    return KeyCode::eKP5;
                case GLFW_KEY_KP_6:
                    return KeyCode::eKP6;
                case GLFW_KEY_KP_7:
                    return KeyCode::eKP7;
                case GLFW_KEY_KP_8:
                    return KeyCode::eKP8;
                case GLFW_KEY_KP_9:
                    return KeyCode::eKP9;
                case GLFW_KEY_KP_DECIMAL:
                    return KeyCode::eKPDecimal;
                case GLFW_KEY_KP_DIVIDE:
                    return KeyCode::eKPDivide;
                case GLFW_KEY_KP_MULTIPLY:
                    return KeyCode::eKPMultiply;
                case GLFW_KEY_KP_SUBTRACT:
                    return KeyCode::eKPSubtract;
                case GLFW_KEY_KP_ADD:
                    return KeyCode::eKPAdd;
                case GLFW_KEY_KP_ENTER:
                    return KeyCode::eKPEnter;
                case GLFW_KEY_KP_EQUAL:
                    return KeyCode::eKPEqual;
                case GLFW_KEY_LEFT_SHIFT:
                    return KeyCode::eLeftShift;
                case GLFW_KEY_LEFT_CONTROL:
                    return KeyCode::eLeftControl;
                case GLFW_KEY_LEFT_ALT:
                    return KeyCode::eLeftAlt;
                case GLFW_KEY_LEFT_SUPER:
                    return KeyCode::eLeftSuper;
                case GLFW_KEY_RIGHT_SHIFT:
                    return KeyCode::eRightShift;
                case GLFW_KEY_RIGHT_CONTROL:
                    return KeyCode::eRightControl;
                case GLFW_KEY_RIGHT_ALT:
                    return KeyCode::eRightAlt;
                case GLFW_KEY_RIGHT_SUPER:
                    return KeyCode::eRightSuper;
                case GLFW_KEY_MENU:
                    return KeyCode::eMenu;
                default:
                    return KeyCode::eUnknown;
            }
        }
    }  // namespace

    GLFWindow::GLFWindow(const WindowInfo& info) noexcept
        : _name(info.name)
        , _extent(info.extent)
        , _fullScreen(info.fullScreen)
        , _windowBordered(info.windowBordered)
        , _exclusiveRefreshRate(info.exclusiveRefreshRate)
        , _exclusiveMonitor(info.exclusiveMonitor)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        if (info.fullScreen)
        {
            glfwWindowHint(GLFW_REFRESH_RATE, info.exclusiveRefreshRate);

            _window =
                glfwCreateWindow(_extent.x, _extent.y, _name.c_str(), exclusiveMonitor(), nullptr);
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

            _window = glfwCreateWindow(_extent.x, _extent.y, _name.c_str(), nullptr, nullptr);
        }

        glfwSetWindowUserPointer(_window, this);

        glfwSetWindowSizeCallback(_window, resizeCallback);
        glfwSetKeyCallback(_window, keyCallback);
    }

    GLFWindow::~GLFWindow() noexcept
    {
        glfwDestroyWindow(_window);
    }

    void GLFWindow::update() noexcept
    {
        glfwPollEvents();
    }

    void GLFWindow::close() noexcept
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    void GLFWindow::addResizeCallback(const ResizeCallback& callback) noexcept
    {
        _resizeCallbacks.push_back(callback);
    }

    void GLFWindow::removeResizeCallback(const ResizeCallback& callback) noexcept
    {
        std::erase_if(_resizeCallbacks,
                      [&](const ResizeCallback& cab)
                      { return cab.callback == callback.callback && cab.data == callback.data; });
    }

    void GLFWindow::addKeyCallback(const KeyCallback& callback) noexcept
    {
        _keyCallbacks.push_back(callback);
    }

    void GLFWindow::removeKeyCallback(const KeyCallback& callback) noexcept
    {
        std::erase_if(_keyCallbacks,
                      [&](const KeyCallback& cab)
                      { return cab.callback == callback.callback && cab.data == callback.data; });
    }

    void GLFWindow::resizeCallback(GLFWwindow* window, int width, int height)
    {
        auto* win = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));

        if (width == 0 || height == 0)
        {
            return;
        }

        win->_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        for (const ResizeCallback& callback : win->_resizeCallbacks)
        {
            callback.callback(callback.data, win->_extent);
        }
    }

    void GLFWindow::keyCallback(
        GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
    {
        auto* win = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));

        KeyAction keyAction = KeyAction::ePress;

        switch (action)
        {
            case GLFW_PRESS:
                keyAction = KeyAction::ePress;
                break;
            case GLFW_RELEASE:
                keyAction = KeyAction::eRelease;
                break;
            case GLFW_REPEAT:
                keyAction = KeyAction::eRepeat;
                break;

            default:
                break;
        }

        auto keyCode = toKeyCode(key);

        for (const KeyCallback& callback : win->_keyCallbacks)
        {
            callback.callback(callback.data, keyCode, keyAction);
        }
    }

    auto GLFWindow::exclusiveMonitor() noexcept -> GLFWmonitor*
    {
        int monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        for (int i = 0; i < monitorCount; i++)
        {
            std::string_view name = glfwGetMonitorName(monitors[i]);

            if (name.compare(_exclusiveMonitor.name) == 0)
            {
                return monitors[i];
            }
        }

        return nullptr;
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
        _extent = extent;
        glfwSetWindowSize(_window, static_cast<int>(extent.x), static_cast<int>(extent.y));
    }

    void GLFWindow::setFullScreen(bool fullScreen) noexcept
    {
        _fullScreen = fullScreen;

        if (_fullScreen)
        {
            glfwSetWindowMonitor(
                _window, exclusiveMonitor(), 0, 0, _extent.x, _extent.y, _exclusiveRefreshRate);
        }
        else
        {
            int xpos, ypos;
            glfwGetWindowPos(_window, &xpos, &ypos);

            glfwSetWindowMonitor(
                _window, nullptr, xpos, ypos, _extent.x, _extent.y, _exclusiveRefreshRate);
        }
    }

    void GLFWindow::setWindowBordered(bool bordered) noexcept
    {
        glfwSetWindowAttrib(_window, GLFW_DECORATED, _windowBordered ? GLFW_TRUE : GLFW_FALSE);
    }

    void GLFWindow::setExclusiveRefreshRate(uint32_t refreshRate) noexcept
    {
        _exclusiveRefreshRate = refreshRate;

        if (_fullScreen)
        {
            glfwSetWindowMonitor(
                _window, exclusiveMonitor(), 0, 0, _extent.x, _extent.y, _exclusiveRefreshRate);
        }
    }

    void GLFWindow::setExclusiveMonitor(Monitor monitor) noexcept
    {
        _exclusiveMonitor = monitor;

        if (_fullScreen)
        {
            glfwSetWindowMonitor(
                _window, exclusiveMonitor(), 0, 0, _extent.x, _extent.y, _exclusiveRefreshRate);
        }
    }

    auto GLFWindow::shouldClose() const noexcept -> bool
    {
        return glfwWindowShouldClose(_window) != 0;
    }

    auto GLFWindow::isMinimized() const noexcept -> bool
    {
        return glfwGetWindowAttrib(_window, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    auto GLFWindow::getMonitorCount() noexcept -> uint32_t
    {
        int count;
        glfwGetMonitors(&count);
        return count;
    }

    auto GLFWindow::getMonitor(uint32_t index) noexcept -> Monitor
    {
        int count = getMonitorCount();

        debugAssert(index < count, "Monitor index out of bounds");

        GLFWmonitor* monitor = glfwGetMonitors(&count)[index];
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        Monitor mon;
        mon.name = glfwGetMonitorName(monitor);
        mon.extent = {mode->width, mode->height};
        mon.refreshRate = mode->refreshRate;

        return mon;
    }

    auto GLFWindow::getMonitors() noexcept -> std::vector<Monitor>
    {
        int count = getMonitorCount();

        std::vector<Monitor> monitors;
        monitors.reserve(count);

        for (int i = 0; i < count; i++)
        {
            monitors.push_back(getMonitor(i));
        }

        return monitors;
    }
}  // namespace exage
