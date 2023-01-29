#ifndef GLFW_INCLUDE_NONE
#    define GLFW_INCLUDE_NONE
#    include "Core/Window.h"
#endif

#include <GLFW/glfw3.h>

#include "GLFWindow.h"

namespace exage
{
    GLFWindow::GLFWindow(const WindowInfo& info) noexcept
        : _name(info.name)
          , _extent(info.extent)
          , _fullScreenMode(info.fullScreenMode)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        switch (info.fullScreenMode)
        {
            case FullScreenMode::eBorderless:
            {
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

                glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

                _window = glfwCreateWindow(
                    mode->width,
                    mode->height,
                    _name.c_str(),
                    monitor,
                    nullptr);

                _extent = {mode->width, mode->height};
            }
            break;

            case FullScreenMode::eExclusive:
            {
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

                glfwWindowHint(GLFW_REFRESH_RATE,
                               static_cast<int>(info.refreshRate));

                _window = glfwCreateWindow(
                    mode->width,
                    mode->height,
                    _name.c_str(),
                    monitor,
                    nullptr);
            }
            break;

            case FullScreenMode::eWindowed:
            {
                _window = glfwCreateWindow(static_cast<int>(info.extent.x),
                                           static_cast<int>(info.extent.y),
                                           _name.c_str(),
                                           nullptr,
                                           nullptr);
            }
            break;
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
                      {
                          return cab.callback == callback.callback
                              && cab.data == callback.data;
                      });
    }

    void GLFWindow::addKeyCallback(const KeyCallback& callback) noexcept
    {
        _keyCallbacks.push_back(callback);
    }

    void GLFWindow::removeKeyCallback(const KeyCallback& callback) noexcept
    {
        std::erase_if(_keyCallbacks,
                      [&](const KeyCallback& cab)
                      {
                          return cab.callback == callback.callback
                              && cab.data == callback.data;
                      });
    }

    void GLFWindow::resizeCallback(GLFWwindow* window, int width, int height)
    {
        auto* win = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
        win->_extent = {static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)};

        for (const ResizeCallback& callback : win->_resizeCallbacks)
        {
            callback.callback(callback.data, win->_extent);
        }
    }

    void GLFWindow::keyCallback(
        GLFWwindow* window,
        int key,
        int /*scancode*/,
        int action,
        int /*mods*/)
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

        auto keyCode = static_cast<KeyCode>(key);

        for (const KeyCallback& callback : win->_keyCallbacks)
        {
            callback.callback(callback.data, keyCode, keyAction);
        }
    }

    auto GLFWindow::getRefreshRate() const noexcept -> uint32_t
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        return mode->refreshRate;
    }

    auto GLFWindow::getFullScreenMode() const noexcept -> FullScreenMode
    {
        return _fullScreenMode;
    }

    void GLFWindow::resize(glm::uvec2 extent) noexcept
    {
        _extent = extent;
        glfwSetWindowSize(
            _window,
            static_cast<int>(extent.x),
            static_cast<int>(extent.y));
    }

    void GLFWindow::setFullScreenMode(FullScreenMode mode) noexcept
    {
        _fullScreenMode = mode;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        switch (mode)
        {
            case FullScreenMode::eBorderless:
            {
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

                glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

                glfwSetWindowMonitor(_window,
                                     monitor,
                                     0,
                                     0,
                                     mode->width,
                                     mode->height,
                                     mode->refreshRate);
            }
            break;

            case FullScreenMode::eExclusive:
            {
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

                glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

                glfwSetWindowMonitor(_window,
                                     monitor,
                                     0,
                                     0,
                                     mode->width,
                                     mode->height,
                                     mode->refreshRate);
            }
            break;

            case FullScreenMode::eWindowed:
            {
                glfwSetWindowMonitor(_window,
                                     nullptr,
                                     0,
                                     0,
                                     static_cast<int>(_extent.x),
                                     static_cast<int>(_extent.y),
                                     GLFW_DONT_CARE);
            }
            break;
        }
    }

    auto GLFWindow::shouldClose() const noexcept -> bool
    {
        return glfwWindowShouldClose(_window) != 0;
    }
} // namespace exage
