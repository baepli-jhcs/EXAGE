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

    ContextCreateInfo createInfo {.api = API::eVulkan,
                                  .windowAPI = exage::WindowAPI::eGLFW,
                                  .optionalWindow = windowReturn.value().get()};

    tl::expected context(Context::create(createInfo));

    REQUIRE(context.has_value());

    SwapchainCreateInfo swapchainCreateInfo {.window = *windowReturn.value()};
    std::unique_ptr swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    REQUIRE(swapchain != nullptr);
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

    ContextCreateInfo createInfo {.api = API::eVulkan,
                                  .windowAPI = exage::WindowAPI::eGLFW,
                                  .optionalWindow = windowReturn.value().get()};

    tl::expected context(Context::create(createInfo));
    REQUIRE(context.has_value());

    QueueCreateInfo queueCreateInfo {.maxFramesInFlight = 2};

    std::unique_ptr queue = context.value()->createQueue(queueCreateInfo);
    REQUIRE(queue != nullptr);

    SwapchainCreateInfo swapchainCreateInfo {.window = *windowReturn.value()};
    std::unique_ptr swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    REQUIRE(swapchain != nullptr);

    std::unique_ptr commandBuffer = context.value()->createCommandBuffer();
    REQUIRE(commandBuffer != nullptr);

    TextureCreateInfo textureCreateInfo {.extent = {1280, 720, 1},
                                         .usage = Texture::UsageFlags::eTransferSource};

    std::shared_ptr texture = context.value()->createTexture(textureCreateInfo);
    REQUIRE(texture != nullptr);

    Context& ctx = *context.value();
    Queue& que = *queue;
    Swapchain& swap = *swapchain;
    CommandBuffer& cmd = *commandBuffer;

    que.startNextFrame();

    std::optional swapError = swap.acquireNextImage(que);
    REQUIRE(!swapError.has_value());

    cmd.begin();

    cmd.textureBarrier(texture,
                       Texture::Layout::eTransferSrc,
                       PipelineStageFlags::eTopOfPipe,
                       PipelineStageFlags::eTransfer,
                       {},
                       AccessFlags::eTransferWrite);

    swap.drawImage(cmd, texture);

    cmd.end();

    QueueSubmitInfo submitInfo {.commandBuffer = cmd};
    que.submit(submitInfo);

    QueuePresentInfo presentInfo {.swapchain = swap};
    std::optional presentError = que.present(presentInfo);
    REQUIRE(!presentError.has_value());

    ctx.waitIdle();
}
