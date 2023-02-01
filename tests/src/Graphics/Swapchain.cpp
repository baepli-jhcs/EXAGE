#include "Graphics/Swapchain.h"

#include <catch2/catch_all.hpp>

#include "Graphics/Queue.h"

TEST_CASE("Creating Graphics Swapchain", "[Swapchain]")
{
    using namespace exage::Graphics;

    constexpr exage::WindowInfo info = {
        .extent = {1280, 720},
        .name = "Main Window",
        .fullScreenMode = exage::FullScreenMode::eWindowed,
    };

    tl::expected<std::unique_ptr<exage::Window>, exage::WindowError> windowReturn =
        exage::Window::create(info, exage::WindowAPI::eGLFW);
    REQUIRE(windowReturn.has_value());

    ContextCreateInfo createInfo{
        .api = API::eVulkan, .windowAPI = exage::WindowAPI::eGLFW, .maxFramesInFlight = 2,
        .optionalWindow = windowReturn.value().get()
    };

    tl::expected context(Context::create(createInfo));

    REQUIRE(context.has_value());

    SwapchainCreateInfo swapchainCreateInfo{
        .window = *windowReturn.value()
    };
    tl::expected swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    REQUIRE(swapchain.has_value());
}

TEST_CASE("Creating Graphics Swapchain and Acquire Next Image", "[Swapchain]")
{
    using namespace exage::Graphics;

    constexpr exage::WindowInfo info = {
        .extent = {1280, 720},
        .name = "Main Window",
        .fullScreenMode = exage::FullScreenMode::eWindowed,
    };

    tl::expected<std::unique_ptr<exage::Window>, exage::WindowError> windowReturn =
        exage::Window::create(info, exage::WindowAPI::eGLFW);
    REQUIRE(windowReturn.has_value());

    ContextCreateInfo createInfo{
        .api = API::eVulkan, .windowAPI = exage::WindowAPI::eGLFW, .maxFramesInFlight = 2,
        .optionalWindow = windowReturn.value().get()
    };

    tl::expected context(Context::create(createInfo));

    REQUIRE(context.has_value());

    SwapchainCreateInfo swapchainCreateInfo{.window = *windowReturn.value()};
    tl::expected swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    REQUIRE(swapchain.has_value());

    Context& ctx = *context.value();
    Queue& que = ctx.getQueue();
    Swapchain& swap = *swapchain.value();
    std::optional error = que.startNextFrame();
    REQUIRE(!error.has_value());

    std::optional swapError = swap.acquireNextImage(que);
    REQUIRE(!swapError.has_value());
}
