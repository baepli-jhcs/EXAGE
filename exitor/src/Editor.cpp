#include <algorithm>
#include <fstream>
#include <limits>

#include "Editor.h"

#include <fmt/format.h>
#include <glm/trigonometric.hpp>

#include "LevelEditor/LevelEditor.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/System/Event.h"
#include "exage/System/Window.h"
#include "utils/files.h"

constexpr static auto WINDOW_API = exage::System::API::eGLFW;
constexpr static auto GRAPHICS_API = exage::Graphics::API::eVulkan;

namespace exitor
{

    Editor::Editor() noexcept
    {
        System::Monitor monitor = getDefaultMonitor(WINDOW_API);
        System::WindowInfo windowInfo {
            .name = "EXitor",
            .extent = {1280, 720},
            .fullScreen = false,
            .windowBordered = true,
            .exclusiveRefreshRate = monitor.refreshRate,
            .exclusiveMonitor = monitor,
            .resizable = true,
        };

        tl::expected windowResult = System::Window::create(windowInfo, WINDOW_API);
        assert(windowResult.has_value());
        _window = std::move(*windowResult);

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

        GUI::ImGui::InitInfo imGuiInfo {
            .context = *_context,
            .window = *_window,
        };
        _imGui = GUI::ImGui::Instance {imGuiInfo};

        _fontManager = GUI::ImGui::FontManager {*_imGui};

        _fontManager->addFont("assets/fonts/SourceSansPro/Regular.ttf", "Source Sans Pro Regular");
        _fontManager->addFont("assets/fonts/SourceSansPro/Bold.ttf", "Source Sans Pro Bold");

        _defaultFont = _fontManager->getFont("Source Sans Pro Regular", 16.F);

        _projectSelector.emplace(*_fontManager);
    }

    Editor::~Editor()
    {
        _context->waitIdle();
    }

    void Editor::run() noexcept
    {
        _timer.reset();

        while (!_window->shouldClose())
        {
            pollEvents(_window->getAPI());
            {
                std::optional event = nextEvent(_window->getAPI());
                while (event.has_value())
                {
                    _imGui->processEvent(*event);
                    event = nextEvent(_window->getAPI());
                }
            }

            if (_window->isIconified())
            {
                continue;
            }

            float deltaTime = _timer.nextFrame();

            tick(deltaTime);
        }
    }

    void Editor::tick(float deltaTime) noexcept
    {
        _context->getQueue().startNextFrame();
        Graphics::CommandBuffer& cmd = _queueCommandRepo->current();
        tl::expected swapError = _swapchain->acquireNextImage();
        if (!swapError.has_value())
        {
            glm::uvec2 extent = _window->getExtent();
            _frameBuffer->resize(extent);
            _swapchain->resize(extent);

            swapError = _swapchain->acquireNextImage();
        }

        cmd.begin();

        handleFonts();

        _fontManager->newFrame();
        _imGui->begin();

        ImGui::PushFont(_defaultFont);
        tickGUI(deltaTime);
        ImGui::PopFont();

        _imGui->end();

        std::shared_ptr<Graphics::Texture> const texture = _frameBuffer->getTexture(0);
        cmd.textureBarrier(texture,
                           Graphics::Texture::Layout::eUndefined,
                           Graphics::Texture::Layout::eColorAttachment,
                           Graphics::PipelineStageFlags::eTopOfPipe,
                           Graphics::PipelineStageFlags::eColorAttachmentOutput,
                           Graphics::Access {},
                           Graphics::AccessFlags::eColorAttachmentWrite,
                           Graphics::QueueOwnership::eUndefined,
                           Graphics::QueueOwnership::eUndefined);

        Graphics::ClearColor const clearColor {.clear = true, .color = {}};
        Graphics::ClearDepthStencil const clearDepthStencil {.clear = false};

        cmd.beginRendering(_frameBuffer, {clearColor}, clearDepthStencil);

        _imGui->renderMainWindow(cmd);

        cmd.endRendering();

        cmd.textureBarrier(texture,
                           Graphics::Texture::Layout::eColorAttachment,
                           Graphics::Texture::Layout::eTransferSrc,
                           Graphics::PipelineStageFlags::eColorAttachmentOutput,
                           Graphics::PipelineStageFlags::eTransfer,
                           Graphics::AccessFlags::eColorAttachmentWrite,
                           Graphics::AccessFlags::eTransferWrite,
                           Graphics::QueueOwnership::eUndefined,
                           Graphics::QueueOwnership::eUndefined);

        _swapchain->drawImage(cmd, texture);

        cmd.end();

        _context->getQueue().submit(cmd);

        _imGui->renderAdditional();

        swapError = _context->getQueue().present(*_swapchain);
    }

    void Editor::handleFonts() noexcept
    {
        if (_projectSelector.has_value())
        {
            _projectSelector->handleFonts();
        }
    }

    void Editor::tickGUI(float deltaTime) noexcept
    {
        if (_projectSelector.has_value())
        {
            std::optional<ProjectReturn> selectedProject = _projectSelector->run();
            if (!selectedProject.has_value())
            {
                return;
            }

            LevelEditorCreateInfo levelEditorCreateInfo {
                .context = _context.get(),
                .project = std::move(selectedProject->project),
                .projectPath = std::move(selectedProject->path),
                .projectDirectory = std::move(selectedProject->directory)};

            _levelEditor.emplace(levelEditorCreateInfo);
            _projectSelector.reset();
        }

        else if (_levelEditor.has_value())
        {
            _levelEditor->run(_queueCommandRepo->current(), deltaTime);
        }
    }
}  // namespace exitor
