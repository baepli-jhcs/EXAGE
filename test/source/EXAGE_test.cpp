#include <catch2/catch_all.hpp>

#include "exage/System/Window.h"

TEST_CASE("Creating GLFW Window", "[Window]")
{
    using namespace exage::System;

    constexpr WindowInfo info = {
        .name = "Test Window",
        .extent = {800, 600},
        .fullScreen = false,
        .windowBordered = true,
    };

    tl::expected<std::unique_ptr<Window>, WindowError> windowReturn =
        Window::create(info, WindowAPI::eGLFW);
    REQUIRE(windowReturn.has_value());
    windowReturn.value()->close();
}
