#include "Core/Core.h"
#include "Graphics/Context.h"
#include "Graphics/HLPD/ImGuiTools.h"
#include "Graphics/Queue.h"

using namespace exage;
using namespace exage::Graphics;

static void resizeCallback(void* data, glm::uvec2 extent);

struct ResizeData
{
    Context& ctx;
    Swapchain& swap;
    TextureCreateInfo& textureCreateInfo;
    tl::expected<std::shared_ptr<Texture>, Error>& texture;
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

    SwapchainCreateInfo swapchainCreateInfo {.window = *windowReturn.value()};
    tl::expected swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    QueueCommandRepoCreateInfo queueCommandRepoCreateInfo {.context = *context.value(),
                                                           .queue = *queue.value()};
    tl::expected queueCommandRepo(QueueCommandRepo::create(queueCommandRepoCreateInfo));

    TextureCreateInfo textureCreateInfo {.extent = {1280, 720, 1},
                                         .usage = Texture::UsageFlags::eTransferSource};

    tl::expected texture = context.value()->createTexture(textureCreateInfo);

    Window& window = *windowReturn.value();
    Context& ctx = *context.value();
    Queue& que = *queue.value();
    Swapchain& swap = *swapchain.value();
    QueueCommandRepo& repo = queueCommandRepo.value();

    ResizeData resizeData {
        .ctx = ctx, .swap = swap, .textureCreateInfo = textureCreateInfo, .texture = texture};

    ResizeCallback callback = {&resizeData, resizeCallback};

    window.addResizeCallback(callback);

    while (!window.shouldClose())
    {
        window.update();

        CommandBuffer& cmd = repo.current();
        std::optional queueError = que.startNextFrame();
        std::optional swapError = swap.acquireNextImage(que);
        std::optional cmdError = cmd.begin();

        TextureBarrier barrier {.texture = texture.value(),
                                .newLayout = Texture::Layout::eTransferSrc,
                                .srcStage = PipelineStageFlags::eTopOfPipe,
                                .dstStage = PipelineStageFlags::eTransfer,
                                .srcAccess = {},
                                .dstAccess = AccessFlags::eTransferWrite};
        cmd.submitCommand(barrier);

        swapError = swap.drawImage(cmd, texture.value());

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
    resizeData->textureCreateInfo.extent = {extent.x, extent.y, 1};

    resizeData->texture = resizeData->ctx.createTexture(resizeData->textureCreateInfo);
}
