#include "exage/Core/Core.h"

#include <FreeImage.h>

#include "exage/Core/Timer.h"
#include "exage/platform/GLFW/GLFWWindow.h"

namespace exage
{
    void init() noexcept
    {
        // Initialize the engine
        System::GLFWWindow::init();

        // Initialize FreeImage
        FreeImage_Initialise();

        Timer::init();
    }
}  // namespace exage
