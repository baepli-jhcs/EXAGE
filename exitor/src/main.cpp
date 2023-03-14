#include "Core/Core.h"
#include "Graphics/Context.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/HLPD/ImGuiTools.h"
#include "Graphics/Queue.h"
#include "glm/gtc/random.hpp"

using namespace exage;
using namespace exage::Graphics;

static void resizeCallback(void* data, glm::uvec2 extent);

struct ResizeData
{
    Context& ctx;
    Swapchain& swap;
    TextureCreateInfo& textureCreateInfo;
    FrameBuffer& frameBuffer;
};

auto main(int argc, char* argv[]) -> int
{
    exage::init();

    constexpr exage::WindowInfo info = {
        .extent = {1280, 720},
        .name = "Main Window",
        .fullScreenMode = exage::FullScreenMode::eWindowed,
    };

    tl::expected windowReturn = Window::create(info, exage::WindowAPI::eGLFW);
    ContextCreateInfo createInfo {.api = API::eVulkan,
                                  .windowAPI = exage::WindowAPI::eGLFW,
                                  .optionalWindow = windowReturn.value().get()};
    tl::expected context(Context::create(createInfo));

    QueueCreateInfo queueCreateInfo {.maxFramesInFlight = 2};
    tl::expected queue = context.value()->createQueue(queueCreateInfo);

    SwapchainCreateInfo swapchainCreateInfo {.window = *windowReturn.value(), .presentMode = PresentMode::eDoubleBufferVSync};
    tl::expected swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    QueueCommandRepoCreateInfo queueCommandRepoCreateInfo {.context = *context.value(),
                                                           .queue = *queue.value()};
    tl::expected queueCommandRepo(QueueCommandRepo::create(queueCommandRepoCreateInfo));

    TextureCreateInfo textureCreateInfo {
        .extent = {1280, 720, 1},
        .usage = Texture::UsageFlags::eTransferSource | Texture::UsageFlags::eColorAttachment};

    tl::expected texture = context.value()->createTexture(textureCreateInfo);

    tl::expected frameBuffer = context.value()->createFrameBuffer(glm::uvec2 {1280, 720});
    std::optional<Error> frameError = frameBuffer.value()->attachColor(texture.value());

    Window& window = *windowReturn.value();
    Context& ctx = *context.value();
    Queue& que = *queue.value();
    Swapchain& swap = *swapchain.value();
    QueueCommandRepo& repo = queueCommandRepo.value();
    FrameBuffer& frame = *frameBuffer.value();

    ResizeData resizeData {
        .ctx = ctx, .swap = swap, .textureCreateInfo = textureCreateInfo, .frameBuffer = frame};

    ResizeCallback callback = {&resizeData, resizeCallback};

    window.addResizeCallback(callback);

    while (!window.shouldClose())
    {
        window.update();

        CommandBuffer& cmd = repo.current();
        std::optional queueError = que.startNextFrame();
        std::optional swapError = swap.acquireNextImage(que);
        std::optional cmdError = cmd.begin();

        std::shared_ptr<Texture> texture = frame.getTexture(0);

        TextureBarrier colorBarrier {
            .texture = texture,
            .newLayout = Texture::Layout::eColorAttachment,
            .srcStage = PipelineStageFlags::eTransfer,
            .dstStage = PipelineStageFlags::eColorAttachmentOutput,
            .srcAccess = AccessFlags::eTransferWrite,
            .dstAccess = AccessFlags::eColorAttachmentWrite,
        };

        cmd.submitCommand(colorBarrier);

        // glm vec4 random clear color
        glm::vec4 clearCol = glm::linearRand(glm::vec4 {0.0f}, glm::vec4 {1.0f});

        BeginRenderingCommand::ClearColor clearColor {.clear = true,
                                                      .color = clearCol};
        BeginRenderingCommand::ClearDepthStencil clearDepthStencil {.clear = false};
        BeginRenderingCommand beginRenderingCommand {.frameBuffer = frameBuffer.value(),
                                                     .clearColors = {clearColor},
                                                     .clearDepth = clearDepthStencil};

        cmd.submitCommand(beginRenderingCommand);

        cmd.submitCommand(EndRenderingCommand {});

        TextureBarrier barrier {.texture = texture,
                                .newLayout = Texture::Layout::eTransferSrc,
                                .srcStage = PipelineStageFlags::eTopOfPipe,
                                .dstStage = PipelineStageFlags::eTransfer,
                                .srcAccess = {},
                                .dstAccess = AccessFlags::eTransferWrite};
        cmd.submitCommand(barrier);

        swapError = swap.drawImage(cmd, texture);

        cmdError = cmd.end();
        QueueSubmitInfo submitInfo {.commandBuffer = cmd};
        queueError = que.submit(submitInfo);
        QueuePresentInfo presentInfo {.swapchain = swap};
        swapError = que.present(presentInfo);
    }

    ctx.waitIdle();
}

static void resizeCallback(void* data, glm::uvec2 extent)
{
    ResizeData* resizeData = static_cast<ResizeData*>(data);
    std::optional<Error> swapError = resizeData->swap.resize(extent);
    std::optional<Error> frameError = resizeData->frameBuffer.resize(extent);
}
