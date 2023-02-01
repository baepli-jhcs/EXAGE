#include "Graphics/Queue.h"

#include <catch2/catch_all.hpp>

TEST_CASE("Creating Graphics Queue", "[Queue]")
{
    using namespace exage::Graphics;

    ContextCreateInfo createInfo{
        .api = API::eVulkan, .windowAPI = exage::WindowAPI::eGLFW, .maxFramesInFlight = 2};

    tl::expected context(Context::create(createInfo));

    REQUIRE(context.has_value());
    REQUIRE(context.value()->getQueue().getAPI() == API::eVulkan);
}
