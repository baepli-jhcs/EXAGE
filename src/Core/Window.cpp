#include "Core/Window.h"

#include "GLFW/GLFWindow.h"

namespace exage
{
    auto Window::create(const WindowInfo& info,
                        WindowAPI api) noexcept -> tl::expected<
        std::unique_ptr<Window>, WindowError>
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
} // namespace exage
