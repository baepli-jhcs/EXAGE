#include "exage/Core/Core.h"

#include <FreeImage.h>

#include "exage/Core/Timer.h"
#include "exage/platform/GLFW/GLFWindow.h"

namespace exage
{
    void init() noexcept
    {
        // Initialize the engine
        System::GLFWindow::init();

        // Initialize FreeImage
        FreeImage_Initialise();

        Timer::init();
    }
}  // namespace exage
