#include "exage/Core/Core.h"

#include <FreeImage.h>

#include "exage/platform/GLFW/GLFWindow.h"

namespace exage
{
    void init()
    {
        // Initialize the engine
        System::GLFWindow::init();

        // Initialize FreeImage
        FreeImage_Initialise();
    }
}  // namespace exage
