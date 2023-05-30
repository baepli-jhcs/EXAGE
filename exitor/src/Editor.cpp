#include <algorithm>

#include "Editor.h"

#include <glm/trigonometric.hpp>

#include "exage/Core/Debug.h"
#include "exage/Core/Window.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Renderer/Renderer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Loader/Converter.h"
#include "exage/Renderer/Scene/Loader/Loader.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Hierarchy.h"

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

        prepareTestScene();
    }

    Editor::~Editor()
    {
        _context->waitIdle();
    }

    void Editor::prepareTestScene() noexcept
    {
        auto cameraEntity = _scene.createEntity();

        auto& camera = _scene.addComponent<Renderer::Camera>(cameraEntity);
        camera.fov = glm::radians(45.0F);

        auto& transform = _scene.addComponent<Transform3D>(cameraEntity);
        transform.position = {0.0F, 0.F, 0.0F};
        transform.rotation = glm::vec3 {0.F, 0.F, 0.F};

        Renderer::setSceneCamera(_scene, cameraEntity);

        auto importResult = Renderer::importAsset("assets/exage/models/sponza/sponza.gltf");
        debugAssume(importResult.has_value(), "Failed to import asset");

        auto saveResult = Renderer::saveAssets(*importResult, "assets/exage/models/exspon", "");

        Renderer::AssetCache assetCache;

        auto commamdBuffer = _context->createCommandBuffer();
        commamdBuffer->begin();

        for (auto& texturePtr : importResult->textures)
        {
            Renderer::Texture& texture = *texturePtr;

            Renderer::TextureUploadOptions uploadOptions {.context = *_context,
                                                          .commandBuffer = *commamdBuffer};

            Renderer::GPUTexture gpuTexture = Renderer::uploadTexture(texture, uploadOptions);
            assetCache.addTexture(gpuTexture);
        }

        for (auto& materialPtr : importResult->materials)
        {
            Renderer::Material& material = *materialPtr;

            Renderer::GPUMaterial gpuMaterial {.path = material.path};
            if (assetCache.hasTexture(material.albedo.texturePath))
            {
                gpuMaterial.albedoTexture = assetCache.getTexture(material.albedo.texturePath);
            }
            if (assetCache.hasTexture(material.emissive.texturePath))
            {
                gpuMaterial.emissiveTexture = assetCache.getTexture(material.emissive.texturePath);
            }
            if (assetCache.hasTexture(material.normal.texturePath))
            {
                gpuMaterial.normalTexture = assetCache.getTexture(material.normal.texturePath);
            }
            if (assetCache.hasTexture(material.metallic.texturePath))
            {
                gpuMaterial.metallicTexture = assetCache.getTexture(material.metallic.texturePath);
            }
            if (assetCache.hasTexture(material.roughness.texturePath))
            {
                gpuMaterial.roughnessTexture =
                    assetCache.getTexture(material.roughness.texturePath);
            }
            if (assetCache.hasTexture(material.occlusion.texturePath))
            {
                gpuMaterial.occlusionTexture =
                    assetCache.getTexture(material.occlusion.texturePath);
            }

            Renderer::GPUMaterial::Data data =
                Renderer::materialDataFromGPUAndCPU(gpuMaterial, material);

            Graphics::BufferCreateInfo bufferCreateInfo {
                .size = sizeof(Renderer::GPUMaterial::Data),
                .mapMode = Graphics::Buffer::MapMode::eMapped,
                .cached = false,
            };

            gpuMaterial.buffer = _context->createBuffer(bufferCreateInfo);
            gpuMaterial.buffer->write(std::as_bytes(std::span(&data, 1)), 0);

            assetCache.addMaterial(gpuMaterial);
        }

        std::vector<Renderer::GPUMesh> gpuMeshes;

        for (auto& meshPtr : importResult->meshes)
        {
            Renderer::Mesh& mesh = *meshPtr;

            Renderer::MeshUploadOptions uploadOptions {.context = *_context,
                                                       .commandBuffer = *commamdBuffer,
                                                       .sceneBuffer = *_sceneBuffer};

            Renderer::GPUMesh gpuMesh = Renderer::uploadMesh(mesh, uploadOptions);

            if (assetCache.hasMaterial(mesh.materialPath))
            {
                gpuMesh.material = assetCache.getMaterial(mesh.materialPath);
            }

            assetCache.addMesh(gpuMesh);
            gpuMeshes.push_back(gpuMesh);
        }

        commamdBuffer->end();
        _context->getQueue().submitTemporary(std::move(commamdBuffer));

        Renderer::AssetSceneImportInfo sceneImportInfo {.meshes = gpuMeshes,
                                                        .rootNodes = importResult->rootNodes};
        Renderer::importScene(sceneImportInfo, _scene);
    }

    void Editor::run() noexcept
    {
        while (!_window->shouldClose())
        {
            pollEvents(_window->getAPI());

            if (_window->isMinimized())
            {
                continue;
            }

            _scene.updateHierarchy();

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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport", &showViewport, ImGuiWindowFlags_AlwaysAutoResize);

        // get the size of the viewport
        ImVec2 viewportWindowSize = ImGui::GetContentRegionAvail();
        if (viewportWindowSize.x < 0.0F)
        {
            viewportWindowSize.x = 0.0F;
        }
        if (viewportWindowSize.y < 0.0F)
        {
            viewportWindowSize.y = 0.0F;
        }

        glm::uvec2 viewportSize = {static_cast<uint32_t>(viewportWindowSize.x),
                                   static_cast<uint32_t>(viewportWindowSize.y)};

        if (ImGui::IsWindowHovered())
        {
        }

        ImGui::Image(_renderer->getFrameBuffer().getTexture(0).get(), viewportWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();

        if (viewportSize != _renderer->getExtent())
        {
            _renderer->resize(viewportSize);
        }

        ImGui::Begin("Render Settings");
        ImGui::End();

        ImGui::End();
    }
}  // namespace exitor
