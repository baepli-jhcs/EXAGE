#include "Editor.h"

#include "exage/Renderer/Renderer.h"
#include "exage/Renderer/Scene/SceneBuffer.h"

constexpr static auto WINDOW_API = exage::WindowAPI::eGLFW;
constexpr static auto GRAPHICS_API = exage::Graphics::API::eVulkan;

namespace exitor
{
    Editor::Editor() noexcept
    {
        Monitor monitor = getDefaultMonitor(WINDOW_API);
        WindowInfo windowInfo {
            .name = "EXitor",
            .extent = {monitor.extent.x * 3 / 4, monitor.extent.y * 3 / 4},
            .fullScreen = false,
            .windowBordered = true,
            .exclusiveRefreshRate = monitor.refreshRate,
            .exclusiveMonitor = monitor,
        };

        tl::expected windowResult = Window::create(windowInfo, WINDOW_API);
        assert(windowResult.has_value());
        _window = std::move(*windowResult);

        _window->setResizeCallback([this](glm::uvec2 extent) { resizeCallback(extent); });

        Graphics::ContextCreateInfo contextInfo {
            .api = GRAPHICS_API,
            .windowAPI = WINDOW_API,
            .optionalWindow = _window.get(),
            .maxFramesInFlight = 2,
        };
        contextInfo.optionalWindow = _window.get();
        contextInfo.api = GRAPHICS_API;

        tl::expected contextResult = Graphics::Context::create(contextInfo);
        assert(contextResult.has_value());
        _context = std::move(*contextResult);

        Graphics::SwapchainCreateInfo swapchainInfo {
            .window = *_window, .presentMode = Graphics::PresentMode::eTripleBufferVSync};
        _swapchain = _context->createSwapchain(swapchainInfo);

        Graphics::FrameBufferCreateInfo frameBufferCreateInfo {};
        frameBufferCreateInfo.extent = _window->getExtent();
        frameBufferCreateInfo.colorAttachments.resize(1);
        frameBufferCreateInfo.colorAttachments[0] = {
            Graphics::Format::eRGBA8,
            Graphics::Texture::UsageFlags::eColorAttachment
                | Graphics::Texture::UsageFlags::eTransferSrc};

        _frameBuffer = _context->createFrameBuffer(frameBufferCreateInfo);

        Graphics::QueueCommandRepoCreateInfo queueCommandRepoInfo {.context = *_context};
        _queueCommandRepo = exage::Graphics::QueueCommandRepo {queueCommandRepoInfo};

        Graphics::ImGuiInitInfo imGuiInfo {
            .context = *_context,
            .window = *_window,
        };
        _imGui = Graphics::ImGuiInstance {imGuiInfo};

        Renderer::SceneBufferCreateInfo sceneBufferCreateInfo {.context = *_context};
        _sceneBuffer = Renderer::SceneBuffer {sceneBufferCreateInfo};

        Renderer::RendererCreateInfo rendererCreateInfo {
            .context = *_context, .sceneBuffer = *_sceneBuffer, .extent = _viewportExtent};
        _renderer = Renderer::Renderer {rendererCreateInfo};
    }

    Editor::~Editor()
    {
        _context->waitIdle();
    }

    void Editor::run() noexcept
    {
        while (!_window->shouldClose())
        {
            _window->update();

            if (_window->isMinimized())
            {
                continue;
            }

            Renderer::copySceneForRenderer(_scene);

            Graphics::CommandBuffer& cmd = _queueCommandRepo->current();
            _context->getQueue().startNextFrame();
            tl::expected swapError = _swapchain->acquireNextImage();
            if (!swapError.has_value())
            {
                continue;
            }
            cmd.begin();

            _renderer->render(cmd, _scene);

            cmd.textureBarrier(_renderer->getFrameBuffer().getTexture(0),
                               Graphics::Texture::Layout::eShaderReadOnly,
                               Graphics::PipelineStageFlags::eColorAttachmentOutput,
                               Graphics::PipelineStageFlags::eFragmentShader,
                               Graphics::AccessFlags::eColorAttachmentWrite,
                               Graphics::AccessFlags::eShaderRead);

            std::shared_ptr<Graphics::Texture> const texture = _frameBuffer->getTexture(0);
            cmd.textureBarrier(texture,
                               Graphics::Texture::Layout::eColorAttachment,
                               Graphics::PipelineStageFlags::eTopOfPipe,
                               Graphics::PipelineStageFlags::eColorAttachmentOutput,
                               Graphics::Access {},
                               Graphics::AccessFlags::eColorAttachmentWrite);

            Graphics::ClearColor const clearColor {.clear = true, .color = {}};
            Graphics::ClearDepthStencil const clearDepthStencil {.clear = false};

            cmd.beginRendering(_frameBuffer, {clearColor}, clearDepthStencil);
            _imGui->begin();

            drawGUI();

            _imGui->end();

            _imGui->renderMainWindow(cmd);

            cmd.endRendering();

            cmd.textureBarrier(texture,
                               Graphics::Texture::Layout::eTransferSrc,
                               Graphics::PipelineStageFlags::eColorAttachmentOutput,
                               Graphics::PipelineStageFlags::eTransfer,
                               Graphics::AccessFlags::eColorAttachmentWrite,
                               Graphics::AccessFlags::eTransferWrite);

            _swapchain->drawImage(cmd, texture);

            cmd.end();

            Graphics::QueueSubmitInfo submitInfo {.commandBuffer = cmd};
            _context->getQueue().submit(submitInfo);

            _imGui->renderAdditional();

            Graphics::QueuePresentInfo presentInfo {.swapchain = *_swapchain};
            swapError = _context->getQueue().present(presentInfo);
        }
    }
    void Editor::resizeCallback(glm::uvec2 extent)
    {
        _frameBuffer->resize(extent);
        _renderer->resize(extent);
    }

    void Editor::drawGUI() noexcept
    {
        bool open = true;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus;

        static ImGuiDockNodeFlags const dockspaceFlags = ImGuiDockNodeFlags_None;

        if ((dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0)
        {
            windowFlags |= ImGuiWindowFlags_NoBackground;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiIO const& io = ImGui::GetIO();
        ImGuiStyle const& style = ImGui::GetStyle();

        ImGui::Begin("DockSpace", &open, windowFlags);

        if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0)
        {
            ImGuiID const dockspaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), dockspaceFlags);
        }

        bool showViewport = true;
        ImVec2 const viewportWindowSize = {static_cast<float>(_viewportExtent.x),
                                           static_cast<float>(_viewportExtent.y)};
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowContentSize(viewportWindowSize);
        ImGui::Begin("Viewport", &showViewport, ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::IsWindowHovered())
        {
        }

        ImGui::Image(_renderer->getFrameBuffer().getTexture(0).get(), viewportWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Render Settings");
        ImGui::End();

        ImGui::End();
    }
}  // namespace exitor
