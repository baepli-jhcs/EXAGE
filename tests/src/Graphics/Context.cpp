#include "Graphics/Context.h"
#include "Vulkan/VulkanContext.h"

#include <catch2/catch_all.hpp>

#include "Core/Window.h"

TEST_CASE("Creating Graphics Context", "[Context]")
{
    using namespace exage::Graphics;

    ContextCreateInfo createInfo{
        .api = API::eVulkan,
        .windowAPI = exage::WindowAPI::eGLFW,
    };

    tl::expected context(Context::create(createInfo));

    REQUIRE(context.has_value());
}

TEST_CASE("Creating Graphics Context and Casting to Vulkan Context", "[Context]")
{
    using namespace exage::Graphics;

    ContextCreateInfo createInfo{
        .api = API::eVulkan, .windowAPI = exage::WindowAPI::eGLFW};

    tl::expected context(Context::create(createInfo));

    REQUIRE(context.has_value());

    auto* vulkanContext = context.value()->as<VulkanContext>();

    REQUIRE(vulkanContext != nullptr);
    REQUIRE(vulkanContext->getAPI() == API::eVulkan);
}
