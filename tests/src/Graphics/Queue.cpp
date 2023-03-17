﻿#include "Graphics/Queue.h"

#include <catch2/catch_all.hpp>

TEST_CASE("Creating Graphics Queue", "[Queue]")
{
    using namespace exage::Graphics;

    ContextCreateInfo createInfo{
        .api = API::eVulkan, .windowAPI = exage::WindowAPI::eGLFW};

    tl::expected context(Context::create(createInfo));
    REQUIRE(context.has_value());

    QueueCreateInfo queueCreateInfo{.maxFramesInFlight = 2};

    std::unique_ptr queue = context.value()->createQueue(queueCreateInfo);
    REQUIRE(queue != nullptr);
}
