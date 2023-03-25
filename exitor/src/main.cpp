#include <random>

#include "Core/Core.h"
#include "Graphics/Context.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/HLPD/ImGuiTools.h"
#include "Graphics/Queue.h"
#include "Graphics/Utils/BufferTypes.h"
#include "Graphics/Utils/QueueCommand.h"
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

    exage::Monitor monitor = exage::getDefaultMonitor(WindowAPI::eGLFW);

    exage::WindowInfo info = {
        .name = "Main Window",
        .extent = {monitor.extent.x * 3 / 4, monitor.extent.y * 3 / 4},
        .fullScreen = false,
        .windowBordered = true,
        .exclusiveRefreshRate = monitor.refreshRate,
        .exclusiveMonitor = monitor,
    };

    tl::expected windowReturn = Window::create(info, exage::WindowAPI::eGLFW);
    glm::uvec2 initialExtent = windowReturn.value()->getExtent();

    ContextCreateInfo createInfo {.api = API::eVulkan,
                                  .windowAPI = exage::WindowAPI::eGLFW,
                                  .optionalWindow = windowReturn.value().get()};
    tl::expected context(Context::create(createInfo));

    SwapchainCreateInfo swapchainCreateInfo {.window = *windowReturn.value(),
                                             .presentMode = PresentMode::eDoubleBufferVSync};
    std::unique_ptr swapchain(context.value()->createSwapchain(swapchainCreateInfo));

    QueueCommandRepoCreateInfo queueCommandRepoCreateInfo {.context = *context.value()};
    auto queueCommandRepo = std::make_unique<QueueCommandRepo>(queueCommandRepoCreateInfo);

    TextureCreateInfo textureCreateInfo {
        .extent = {initialExtent.x, initialExtent.y, 1},
        .usage = Texture::UsageFlags::eTransferSource | Texture::UsageFlags::eColorAttachment};

    std::shared_ptr texture = context.value()->createTexture(textureCreateInfo);

    std::shared_ptr frameBuffer = context.value()->createFrameBuffer(initialExtent);
    frameBuffer->attachColor(texture);

    Window& window = *windowReturn.value();
    Context& ctx = *context.value();
    Swapchain& swap = *swapchain;
    QueueCommandRepo& repo = *queueCommandRepo;
    FrameBuffer& frame = *frameBuffer;

    ImGuiInitInfo imguiInitInfo {.context = ctx, .window = window};
    ImGuiInstance imgui(imguiInitInfo);

    ResizeData resizeData {
        .ctx = ctx, .swap = swap, .textureCreateInfo = textureCreateInfo, .frameBuffer = frame};
    ResizeCallback callback = {&resizeData, resizeCallback};
    window.addResizeCallback(callback);

    glm::uvec2 cpuTextureExtent = {500, 500};
    DynamicFixedBufferCreateInfo textureBufferCreateInfo {
        .context = ctx,
        .size = cpuTextureExtent.x * cpuTextureExtent.y * 4,
        .cached = true,
        .useStagingBuffer = false};
    DynamicFixedBuffer cpuTextureBuffer {textureBufferCreateInfo};

    TextureCreateInfo cpuTextureCreateInfo {
        .extent = {cpuTextureExtent.x, cpuTextureExtent.y, 1},
        .usage = Texture::UsageFlags::eTransferDestination,
    };
    std::shared_ptr cpuTexture = ctx.createTexture(cpuTextureCreateInfo);
    std::vector<uint8_t> cpuTextureData(cpuTextureExtent.x * cpuTextureExtent.y * 4);

    while (!window.shouldClose())
    {
        window.update();

        if (window.isMinimized())
        {
            continue;
        }

        CommandBuffer& cmd = repo.current();
        ctx.getQueue().startNextFrame();
        std::optional swapError = swap.acquireNextImage();
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

        for (uint32_t i = 0; i < cpuTextureExtent.x * cpuTextureExtent.y; i++)
        {
            cpuTextureData[i * 4 + 0] = 50;
            cpuTextureData[i * 4 + 1] = 50;
            cpuTextureData[i * 4 + 2] = 200;
            cpuTextureData[i * 4 + 3] = 255;
        }

        std::span<std::byte> cpuTextureDataSpan {
            reinterpret_cast<std::byte*>(cpuTextureData.data()), cpuTextureData.size()};
        cpuTextureBuffer.write(cpuTextureDataSpan, 0);
        cpuTextureBuffer.update(cmd);

        cmd.bufferBarrier(cpuTextureBuffer.currentHost(),
                          PipelineStageFlags::eTransfer,
                          PipelineStageFlags::eTransfer,
                          AccessFlags::eTransferWrite,
                          AccessFlags::eTransferRead);

        cmd.textureBarrier(cpuTexture,
                           Texture::Layout::eTransferDst,  // new layout
                           PipelineStageFlags::eTransfer,
                           PipelineStageFlags::eTransfer,
                           AccessFlags::eTransferWrite,
                           AccessFlags::eTransferRead);

        cmd.copyBufferToTexture(cpuTextureBuffer.currentHost(),
                                cpuTexture,
                                /*srcOffset*/ 0,
                                /*dstOffset*/ glm::uvec3 {0, 0, 0},
                                /*dstMipLevel*/ 0,
                                /*dstFirstLayer*/ 0,
                                /*dstNumLayers*/ 1,
                                /*extent*/ {cpuTextureExtent.x, cpuTextureExtent.y, 1});

        cmd.textureBarrier(cpuTexture,
                           Texture::Layout::eShaderReadOnly,
                           PipelineStageFlags::eTransfer,
                           PipelineStageFlags::eFragmentShader,
                           AccessFlags::eTransferWrite,
                           AccessFlags::eShaderRead);

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

        bool showDemoWindow = true;
        ImGui::ShowDemoWindow(&showDemoWindow);

        bool showTextureWindow = true;
        ImVec2 textureWindowSize = {static_cast<float>(cpuTextureExtent.x),
                                    static_cast<float>(cpuTextureExtent.y)};
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowContentSize(textureWindowSize);
        ImGui::Begin("Texture", &showTextureWindow, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image(cpuTexture.get(), textureWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();

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
        ctx.getQueue().submit(submitInfo);
        QueuePresentInfo presentInfo {.swapchain = swap};
        swapError = ctx.getQueue().present(presentInfo);
    }

    ctx.waitIdle();
}

static void resizeCallback(void* data, glm::uvec2 extent)
{
    ResizeData* resizeData = static_cast<ResizeData*>(data);
    resizeData->swap.resize(extent);
    resizeData->frameBuffer.resize(extent);
}
