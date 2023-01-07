
#include "Core/Window.h"

#include <catch2/catch_all.hpp>

TEST_CASE("Creating GLFW Window", "[Window]")
{
    using namespace exage;

    constexpr WindowInfo info = {
        .extent = {800, 600},
        .name = "Test Window",
        .fullScreenMode = FullScreenMode::eWindowed,
    };

    std::unique_ptr<Window> window(Window::create(info, WindowAPI::eGLFW));
    REQUIRE(window != nullptr);
    window->close();
}
