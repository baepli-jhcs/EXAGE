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
    std::unique_ptr queue = context.value()->createQueue(queueCreateInfo);

    SwapchainCreateInfo swapchainCreateInfo {.window = *windowReturn.value(),
                                             .presentMode = PresentMode::eDoubleBufferVSync};
    std::unique_ptr swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    QueueCommandRepoCreateInfo queueCommandRepoCreateInfo {.context = *context.value(),
                                                           .queue = *queue};
    auto queueCommandRepo = std::make_unique<QueueCommandRepo>(queueCommandRepoCreateInfo);

    TextureCreateInfo textureCreateInfo {
        .extent = {1280, 720, 1},
        .usage = Texture::UsageFlags::eTransferSource | Texture::UsageFlags::eColorAttachment};

    std::shared_ptr texture = context.value()->createTexture(textureCreateInfo);

    std::shared_ptr frameBuffer = context.value()->createFrameBuffer(glm::uvec2 {1280, 720});
    frameBuffer->attachColor(texture);

    Window& window = *windowReturn.value();
    Context& ctx = *context.value();
    Queue& que = *queue;
    Swapchain& swap = *swapchain;
    QueueCommandRepo& repo = *queueCommandRepo;
    FrameBuffer& frame = *frameBuffer;

    ImGuiInitInfo imguiInitInfo {.context = ctx, .queue = que, .window = window};
    ImGuiInstance imgui(imguiInitInfo);

    ResizeData resizeData {
        .ctx = ctx, .swap = swap, .textureCreateInfo = textureCreateInfo, .frameBuffer = frame};
    ResizeCallback callback = {&resizeData, resizeCallback};
    window.addResizeCallback(callback);

    while (!window.shouldClose())
    {
        window.update();

        CommandBuffer& cmd = repo.current();
        que.startNextFrame();
        std::optional swapError = swap.acquireNextImage(que);
        if (swapError.has_value())
        {
            continue;
        }

        std::shared_ptr<Texture> texture = frame.getTexture(0);
        cmd.begin();

        cmd.textureBarrier(texture,
                           Texture::Layout::eColorAttachment,
                           PipelineStageFlags::eTransfer,
                           PipelineStageFlags::eColorAttachmentOutput,
                           AccessFlags::eTransferWrite,
                           AccessFlags::eColorAttachmentWrite);

        // glm vec4 random clear color
        glm::vec4 clearCol = glm::linearRand(glm::vec4 {0.0f}, glm::vec4 {1.0f});

        ClearColor clearColor {.clear = true, .color = clearCol};
        ClearDepthStencil clearDepthStencil {.clear = false};

        cmd.beginRendering(frameBuffer, {clearColor}, clearDepthStencil);

        imgui.begin();

        bool open = true;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus;

        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            windowFlags |= ImGuiWindowFlags_NoBackground;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui::Begin("DockSpace", &open, windowFlags);

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }

        ImGui::End();

        imgui.end();

        imgui.renderMainWindow(cmd);
        cmd.endRendering();

        cmd.textureBarrier(texture,
                           Texture::Layout::eTransferSrc,
                           PipelineStageFlags::eTopOfPipe,
                           PipelineStageFlags::eTransfer,
                           {},
                           AccessFlags::eTransferWrite);

        swap.drawImage(cmd, texture);

        cmd.end();

        imgui.renderAdditional();

        QueueSubmitInfo submitInfo {.commandBuffer = cmd};
        que.submit(submitInfo);
        QueuePresentInfo presentInfo {.swapchain = swap};
        swapError = que.present(presentInfo);
    }

    ctx.waitIdle();
}

static void resizeCallback(void* data, glm::uvec2 extent)
{
    ResizeData* resizeData = static_cast<ResizeData*>(data);
    resizeData->swap.resize(extent);
    resizeData->frameBuffer.resize(extent);
}
