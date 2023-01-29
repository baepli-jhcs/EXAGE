#include "Graphics/Context.h"

#include <catch2/catch_all.hpp>

#include "Core/Window.h"

TEST_CASE("Creating Graphics Context", "[Window]")
{
    using namespace exage::Graphics;

    tl::expected context(
        Context::create(API::eVulkan, exage::WindowAPI::eGLFW));

    REQUIRE(context.has_value());
}
