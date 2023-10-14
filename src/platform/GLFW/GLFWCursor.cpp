#include <cstddef>

#include "exage/platform/GLFW/GLFWCursor.h"

#include <GLFW/glfw3.h>

#include "exage/System/Window.h"
#include "exage/platform/GLFW/GLFWWindow.h"

namespace exage::System
{
    GLFWCursor::GLFWCursor(StandardCursor standardCursor) noexcept
    {
        int glfwCursor = 0;
        switch (standardCursor)
        {
            case StandardCursor::eArrow:
                glfwCursor = GLFW_ARROW_CURSOR;
                break;
            case StandardCursor::eIBeam:
                glfwCursor = GLFW_IBEAM_CURSOR;
                break;
            case StandardCursor::eCrosshair:
                glfwCursor = GLFW_CROSSHAIR_CURSOR;
                break;
            case StandardCursor::eHand:
                glfwCursor = GLFW_HAND_CURSOR;
                break;
            case StandardCursor::eHResize:
                glfwCursor = GLFW_HRESIZE_CURSOR;
                break;
            case StandardCursor::eVResize:
                glfwCursor = GLFW_VRESIZE_CURSOR;
                break;
            default:
                break;
        }

        _cursor = glfwCreateStandardCursor(glfwCursor);
    }

    GLFWCursor::GLFWCursor(glm::uvec2 extent, const uint8_t* pixels, glm::uvec2 hotspot) noexcept
    {
        std::vector<uint8_t> localPixels(static_cast<size_t>(extent.x) * extent.y * 4);
        std::memcpy(localPixels.data(), pixels, localPixels.size());

        GLFWimage image;
        image.width = static_cast<int>(extent.x);
        image.height = static_cast<int>(extent.y);
        image.pixels = localPixels.data();

        _cursor =
            glfwCreateCursor(&image, static_cast<int>(hotspot.x), static_cast<int>(hotspot.y));
    }

    GLFWCursor::~GLFWCursor()
    {
        glfwDestroyCursor(_cursor);
    }

    void GLFWCursor::setCurrent(GLFWCursor* cursor) noexcept
    {
        for (auto&& [window, exageWindow] : GLFWWindow::getWindowMap())
        {
            glfwSetCursor(window, cursor->_cursor);
        }
    }
}  // namespace exage::System