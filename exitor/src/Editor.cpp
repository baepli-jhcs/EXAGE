#include <algorithm>
#include <fstream>
#include <limits>

#include "Editor.h"

#include <fmt/format.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>

#include "MousePicking/MousePicking.h"
#include "Stages/AssetImport.h"
#include "exage/Core/Debug.h"
#include "exage/Core/Event.h"
#include "exage/Core/Window.h"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Projects/Level.h"
#include "exage/Renderer/Renderer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Loader/AssetFile.h"
#include "exage/Renderer/Scene/Loader/Converter.h"
#include "exage/Renderer/Scene/Loader/Loader.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/Utils/Perspective.h"
#include "exage/Scene/Hierarchy.h"
#include "exage/utils/math.h"
#include "exage/utils/string.h"
#include "imgui.h"
#include "utils/files.h"

constexpr static auto WINDOW_API = exage::WindowAPI::eGLFW;
constexpr static auto GRAPHICS_API = exage::Graphics::API::eVulkan;

namespace exitor
{

    Editor::Editor() noexcept
    {
        Monitor monitor = getDefaultMonitor(WINDOW_API);
        WindowInfo windowInfo {
            .name = "EXitor",
            .extent = {monitor.extent.x / 2, monitor.extent.y / 2},
            .fullScreen = false,
            .windowBordered = true,
            .exclusiveRefreshRate = monitor.refreshRate,
            .exclusiveMonitor = monitor,
            .resizable = true,
        };

        tl::expected windowResult = Window::create(windowInfo, WINDOW_API);
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

        Graphics::SamplerCreateInfo samplerCreateInfo {};
        samplerCreateInfo.anisotropy = Graphics::Sampler::Anisotropy::e16;
        samplerCreateInfo.filter = Graphics::Sampler::Filter::eLinear;
        samplerCreateInfo.mipmapMode = Graphics::Sampler::MipmapMode::eLinear;

        _sampler = _context->createSampler(samplerCreateInfo);

        Graphics::ImGuiInitInfo imGuiInfo {
            .context = *_context,
            .window = *_window,
        };
        _imGui = Graphics::ImGuiInstance {imGuiInfo};

        _fontManager = Renderer::FontManager {*_imGui};

        _fontManager->addFont("assets/fonts/SourceSansPro/Regular.ttf", "Source Sans Pro Regular");
        _fontManager->addFont("assets/fonts/SourceSansPro/Bold.ttf", "Source Sans Pro Bold");

        _defaultFont = _fontManager->getFont("Source Sans Pro Regular", 16.F);

        _projectSelector = ProjectSelector {*_fontManager};

        _textureViewer.emplace(*_context, _assetCache);

        Renderer::SceneBufferCreateInfo sceneBufferCreateInfo {.context = *_context};
        _sceneBuffer = Renderer::SceneBuffer {sceneBufferCreateInfo};

        Renderer::RendererCreateInfo rendererCreateInfo {.context = *_context,
                                                         .sceneBuffer = *_sceneBuffer,
                                                         .assetCache = _assetCache,
                                                         .extent = _viewportExtent};
        _renderer = Renderer::Renderer {rendererCreateInfo};

        _renderTexture.sampler = _sampler;

        _componentEditor.setMeshSelectionCallback([this](const std::string& path) -> void
                                                  { onComponentMeshSelection(path); });
        _contentBrowser.setCallbacks(*this);

        createDefaultAssets();
    }

    Editor::~Editor()
    {
        _context->waitIdle();
    }

    void Editor::createDefaultAssets() noexcept {}

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

                    if (_window->getID() == event->pertainingID)
                    {
                        if (auto* windowResized = std::get_if<Events::WindowResized>(&event->data))
                        {
                            resizeCallback(windowResized->extent);
                        }
                    }

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

    void Editor::onComponentMeshSelection(const std::string& path) noexcept
    {
        if (_level->meshPaths.contains(path))
        {
            return;
        }

        _level->meshPaths.insert(path);

        // Load mesh
        auto truePath = getTruePath(path, _projectDirectory);
        auto meshReturn = Renderer::loadMesh(truePath);
        debugAssume(meshReturn.has_value(), "Failed to load mesh");

        Renderer::StaticMesh& mesh = *meshReturn;

        if (!mesh.materialPath.empty())
        {
            _level->materialPaths.insert(mesh.materialPath);

            // Load material
            auto materialTruePath = getTruePath(mesh.materialPath, _projectDirectory);
            auto materialReturn = Renderer::loadMaterial(materialTruePath);

            debugAssume(materialReturn.has_value(), "Failed to load material");

            Renderer::Material& material = *materialReturn;
            auto eachTexture = [&](const std::string& texturePath) -> void
            {
                if (!texturePath.empty())
                {
                    _level->texturePaths.insert(texturePath);

                    // Load texture
                    auto textureTruePath = getTruePath(texturePath, _projectDirectory);
                    auto textureReturn = Renderer::loadTexture(textureTruePath);

                    debugAssume(textureReturn.has_value(), "Failed to load texture");

                    Renderer::Texture& texture = *textureReturn;
                    // Upload texture
                    Renderer::TextureUploadOptions uploadOptions {
                        .context = *_context,
                        .commandBuffer = _queueCommandRepo->current(),
                        .useCompressedFormat = false};
                    Renderer::GPUTexture gpuTexture =
                        Renderer::uploadTexture(texture, uploadOptions);

                    _assetCache.addTexture(gpuTexture);
                }
            };

            eachTexture(material.albedoTexturePath);
            eachTexture(material.emissiveTexturePath);
            eachTexture(material.normalTexturePath);
            eachTexture(material.metallicTexturePath);
            eachTexture(material.roughnessTexturePath);
            eachTexture(material.occlusionTexturePath);

            // Upload material
            Renderer::GPUMaterial gpuMaterial {.path = material.path};
            auto eachTexture2 = [&](Renderer::GPUTexture& texture, const std::string& texturePath)
            {
                if (!texturePath.empty() && _assetCache.hasTexture(texturePath))
                {
                    texture = _assetCache.getTexture(texturePath);
                }
                else
                {
                    texture = _defaultTexture;
                }
            };

            eachTexture2(gpuMaterial.albedoTexture, material.albedoTexturePath);
            eachTexture2(gpuMaterial.emissiveTexture, material.emissiveTexturePath);
            eachTexture2(gpuMaterial.normalTexture, material.normalTexturePath);
            eachTexture2(gpuMaterial.metallicTexture, material.metallicTexturePath);
            eachTexture2(gpuMaterial.roughnessTexture, material.roughnessTexturePath);
            eachTexture2(gpuMaterial.occlusionTexture, material.occlusionTexturePath);

            Renderer::GPUMaterial::Data data =
                Renderer::materialDataFromGPUAndCPU(gpuMaterial, material);

            Graphics::BufferCreateInfo bufferCreateInfo {
                .size = sizeof(Renderer::GPUMaterial::Data),
                .mapMode = Graphics::Buffer::MapMode::eMapped,
                .cached = false,
            };

            gpuMaterial.buffer = _context->createBuffer(bufferCreateInfo);
            gpuMaterial.buffer->write(std::as_bytes(std::span(&data, 1)), 0);

            _assetCache.addMaterial(gpuMaterial);
        }

        // Upload mesh
        Renderer::MeshUploadOptions meshUploadOptions {
            .context = *_context,
            .commandBuffer = _queueCommandRepo->current(),
            .sceneBuffer = *_sceneBuffer,
        };

        Renderer::GPUStaticMesh gpuMesh = Renderer::uploadMesh(mesh, meshUploadOptions);
        if (!mesh.materialPath.empty() && _assetCache.hasMaterial(mesh.materialPath))
        {
            gpuMesh.material = _assetCache.getMaterial(mesh.materialPath);
        }
        else
        {
            gpuMesh.material = _defaultMaterial;
        }

        _assetCache.addMesh(gpuMesh);
    }

    void Editor::tick(float deltaTime) noexcept
    {
        // _scene.updateHierarchy();

        _context->getQueue().startNextFrame();
        Graphics::CommandBuffer& cmd = _queueCommandRepo->current();
        tl::expected swapError = _swapchain->acquireNextImage();
        if (!swapError.has_value())
        {
            return;
        }

        cmd.begin();

        if (_project && _level)
        {
            _level->scene.updateHierarchy();

            Renderer::copySceneForRenderer(_level->scene);
            _renderer->render(cmd, _level->scene);
        }

        _imGui->begin();
        drawGUI(deltaTime);
        _imGui->end();

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

    void Editor::resizeCallback(glm::uvec2 extent) noexcept
    {
        _frameBuffer->resize(extent);
        _renderer->resize(extent);
        _swapchain->resize(extent);
    }

    void Editor::drawGUI(float deltaTime) noexcept
    {
        ImGui::PushFont(_defaultFont);

        if (!_project)
        {
            drawProjectSelector();

            ImGui::PopFont();
            return;
        }

        Scene& scene = _level->scene;

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

        bool shouldCloseProject = false;

        bool openModelMenu = false;
        bool openTextureMenu = false;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("General"))
            {
                if (ImGui::MenuItem("New Project"))
                {
                    shouldCloseProject = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save Level"))
                {
                    if (_editorCameraEntity != entt::null)
                    {
                        _level->scene.destroyEntity(_editorCameraEntity);
                    }

                    Projects::Level level = Projects::serializeLevel(*_level);
                    auto truePath = getTruePath(level.path, _projectDirectory);
                    auto saveResult = Projects::saveLevel(truePath, level);

                    createEditorCamera();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets"))
            {
                if (ImGui::MenuItem("Import Model"))
                {
                    openModelMenu = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Import Texture"))
                {
                    openTextureMenu = true;
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                const char* textureViewerName = "Open Texture Viewer";
                if (_textureViewer->isOpened())
                {
                    textureViewerName = "Close Texture Viewer";
                }

                if (ImGui::MenuItem(textureViewerName))
                {
                    _textureViewer->setOpened(!_textureViewer->isOpened());
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        if (openModelMenu)
        {
            ImGui::OpenPopup(IMPORT_MODEL_ID);
        }

        if (openTextureMenu)
        {
            ImGui::OpenPopup(IMPORT_TEXTURE_ID);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(
            "Viewport", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        ImVec2 currentMousePosIM = ImGui::GetMousePos();
        glm::vec2 currentMousePos = {currentMousePosIM.x, currentMousePosIM.y};

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
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsKeyDown(ImGuiKey_LeftAlt))
            {
                auto camera = Renderer::getSceneCamera(scene);
                auto& transform = scene.getComponent<Transform3D>(camera);
                auto& cameraComponent = scene.getComponent<Renderer::Camera>(camera);

                // Editor camera movement
                glm::vec3 cameraMovement {0.0F, 0.0F, 0.0F};
                if (ImGui::IsKeyDown(ImGuiKey_W))
                {
                    cameraMovement.z += 1.0F;
                }
                if (ImGui::IsKeyDown(ImGuiKey_S))
                {
                    cameraMovement.z -= 1.0F;
                }

                if (ImGui::IsKeyDown(ImGuiKey_A))
                {
                    cameraMovement.x -= 1.0F;
                }
                if (ImGui::IsKeyDown(ImGuiKey_D))
                {
                    cameraMovement.x += 1.0F;
                }

                if (ImGui::IsKeyDown(ImGuiKey_Q))
                {
                    cameraMovement.y -= 1.0F;
                }
                if (ImGui::IsKeyDown(ImGuiKey_E))
                {
                    cameraMovement.y += 1.0F;
                }

                constexpr float margin = 0.01F;
                if (glm::length(cameraMovement) > margin)
                {
                    constexpr static float cameraSpeed = 10.0F;
                    cameraMovement = glm::normalize(cameraMovement) * cameraSpeed * deltaTime;

                    transform.position +=
                        transform.globalRotation.getForwardVector() * cameraMovement.z;
                    transform.position +=
                        transform.globalRotation.getRightVector() * cameraMovement.x;
                    transform.position += transform.globalRotation.getUpVector() * cameraMovement.y;
                }

                // Editor camera rotation
                glm::vec2 mouseDelta = currentMousePos - _lastMousePosition;
                constexpr float mouseSensitivity = 0.01F;
                mouseDelta *= mouseSensitivity;

                std::optional<glm::vec3> euler = transform.rotation.getEuler();
                if (!euler.has_value())
                {
                    euler = {glm::vec3 {0.0F}};
                }

                euler->x += mouseDelta.y;
                euler->y += mouseDelta.x;

                euler->x = std::clamp(euler->x, -glm::half_pi<float>(), glm::half_pi<float>());

                if (euler->y > glm::pi<float>())
                {
                    euler->y -= glm::two_pi<float>();
                }
                else if (euler->y < -glm::pi<float>())
                {
                    euler->y += glm::two_pi<float>();
                }

                transform.rotation = Rotation3D {euler.value(), RotationType::ePitchYawRoll};

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    glm::mat4 viewMatrix =
                        transform.globalRotation.getViewMatrix(transform.globalPosition);
                    glm::mat4 projectionMatrix = Renderer::perspectiveProject(
                        cameraComponent.fov,
                        static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y),
                        cameraComponent.near,
                        cameraComponent.far,
                        true);

                    auto windowPos = ImGui::GetWindowPos();
                    glm::vec2 mousePos = {currentMousePos.x - windowPos.x,
                                          currentMousePos.y - windowPos.y};

                    std::cout << "Mouse pos: " << mousePos.x << ", " << mousePos.y << std::endl;

                    float x = (2.0F * mousePos.x) / viewportSize.x - 1.0F;
                    float y = 1.0F - (2.0F * mousePos.y) / viewportSize.y;

                    glm::vec2 ndc = {x, y};

                    std::cout << "NDC: " << ndc.x << ", " << ndc.y << std::endl;

                    _selectedEntity = getSelectedEntity(
                        _assetCache, scene, _editorCameraEntity, viewMatrix, projectionMatrix, ndc);

                    std::cout << "Selected entity: " << static_cast<uint32_t>(_selectedEntity)
                              << std::endl;
                }
            }
        }

        _renderTexture.texture = _renderer->getFrameBuffer().getTexture(0);
        ImGui::Image(&_renderTexture, viewportWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();

        if (viewportSize != _renderer->getExtent())
        {
            _renderer->resize(viewportSize);
        }

        ImGui::Begin("Render Info");
        ImGui::Text("FPS: %f", 1.0F / deltaTime);
        ImGui::Text("Frame time: %f", deltaTime);

        // Camera position
        auto camera = Renderer::getSceneCamera(scene);
        auto& transform = scene.getComponent<Transform3D>(camera);

        ImGui::Text("Camera position: %f, %f, %f",
                    transform.position.x,
                    transform.position.y,
                    transform.position.z);

        // Camera rotation
        std::optional<glm::vec3> euler = transform.rotation.getEuler();
        if (!euler.has_value())
        {
            euler = {glm::vec3 {0.0F}};
        }

        ImGui::Text("Camera rotation: %f, %f, %f", euler->x, euler->y, euler->z);

        ImGui::End();

        exage::Entity selectedEntity = _hierarchyPanel.draw(scene, _editorCameraEntity);
        entt::id_type type = _componentList.draw(scene, selectedEntity);
        _componentEditor.draw(scene, selectedEntity, type, *_project);

        _contentBrowser.render(_projectDirectory, *_project);

        auto importResult = _assetImport.importAssetScreen(_projectDirectory, *_project);
        if (importResult.first)
        {
            processModelImport(*importResult.first);
        }
        if (importResult.second)
        {
            processTextureImport(*importResult.second);
        }

        _textureViewer->draw(_queueCommandRepo->current(), *_project, _projectDirectory);

        ImGui::End();

        ImGui::PopFont();

        _lastMousePosition = currentMousePos;

        if (shouldCloseProject)
        {
            _project = {};
            _componentList.reset();
            closeLevel();
        }
    }

    void Editor::drawProjectSelector() noexcept
    {
        auto returnValue = _projectSelector->run();
        if (returnValue.has_value())
        {
            _project = returnValue->project;
            _projectPath = returnValue->path;
            _projectDirectory = returnValue->directory;
        }

        if (!_project)
        {
            return;
        }

        const auto& levelPath = _project->defaultLevelPath;
        auto truePath = getTruePath(levelPath, _projectDirectory);

        auto levelResult = Projects::loadLevel(truePath);
        if (!levelResult.has_value())
        {
            _project = {};
            return;
        }

        _level = Projects::deserializeLevel(*levelResult);

        loadLevelAssets();
        createEditorCamera();
    }

    void Editor::createEditorCamera() noexcept
    {
        _editorCameraEntity = _level->scene.createEntity();
        auto& camera = _level->scene.addComponent<Renderer::Camera>(_editorCameraEntity);
        camera.fov = glm::radians(45.0F);

        auto& transform = _level->scene.addComponent<Transform3D>(_editorCameraEntity);
        transform.position = {0.0F, 0.F, 0.0F};
        transform.rotation = Rotation3D {{0, 0, 0}, RotationType::ePitchYawRoll};

        Renderer::setSceneCamera(_level->scene, _editorCameraEntity);
    }

    void Editor::loadLevelAssets() noexcept
    {
        // Load textures
        for (const auto& texturePath : _level->texturePaths)
        {
            loadTexture(texturePath);
        }

        // Load materials
        for (const auto& materialPath : _level->materialPaths)
        {
            loadMaterial(materialPath);
        }

        // Load meshes
        for (const auto& meshPath : _level->meshPaths)
        {
            loadMesh(meshPath);
        }
    }

    void Editor::loadTexture(const std::string& path) noexcept
    {
        auto truePath = getTruePath(path, _projectDirectory);
        auto textureReturn = Renderer::loadTexture(truePath);
        debugAssume(textureReturn.has_value(), "Failed to load texture");

        // Renderer::Texture& texture = *textureReturn;

        // auto newCommandBuffer = _context->createCommandBuffer();
        // newCommandBuffer->begin();

        // // Upload texture
        // Renderer::TextureUploadOptions uploadOptions {
        //     .context = *_context, .commandBuffer = *newCommandBuffer, .useCompressedFormat =
        //     false};
        // Renderer::GPUTexture gpuTexture = Renderer::uploadTexture(texture, uploadOptions);

        // newCommandBuffer->end();

        // _context->getQueue().submitTemporary(std::move(newCommandBuffer));

        Renderer::Texture& texture = *textureReturn;
        // Upload texture
        Renderer::TextureUploadOptions uploadOptions {.context = *_context,
                                                      .commandBuffer = _queueCommandRepo->current(),
                                                      .useCompressedFormat = false};
        Renderer::GPUTexture gpuTexture = Renderer::uploadTexture(texture, uploadOptions);

        _assetCache.addTexture(gpuTexture);
    }

    void Editor::loadMaterial(const std::string& path) noexcept
    {
        auto truePath = getTruePath(path, _projectDirectory);
        auto materialReturn = Renderer::loadMaterial(truePath);

        debugAssume(materialReturn.has_value(), "Failed to load material");

        Renderer::Material& material = *materialReturn;

        // Upload material
        Renderer::GPUMaterial gpuMaterial {.path = material.path};
        if (_assetCache.hasTexture(material.albedoTexturePath))
        {
            gpuMaterial.albedoTexture = _assetCache.getTexture(material.albedoTexturePath);
        }
        if (_assetCache.hasTexture(material.emissiveTexturePath))
        {
            gpuMaterial.emissiveTexture = _assetCache.getTexture(material.emissiveTexturePath);
        }
        if (_assetCache.hasTexture(material.normalTexturePath))
        {
            gpuMaterial.normalTexture = _assetCache.getTexture(material.normalTexturePath);
        }
        if (_assetCache.hasTexture(material.metallicTexturePath))
        {
            gpuMaterial.metallicTexture = _assetCache.getTexture(material.metallicTexturePath);
        }
        if (_assetCache.hasTexture(material.roughnessTexturePath))
        {
            gpuMaterial.roughnessTexture = _assetCache.getTexture(material.roughnessTexturePath);
        }
        if (_assetCache.hasTexture(material.occlusionTexturePath))
        {
            gpuMaterial.occlusionTexture = _assetCache.getTexture(material.occlusionTexturePath);
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

        _assetCache.addMaterial(gpuMaterial);
    }

    void Editor::loadMesh(const std::string& path) noexcept
    {
        auto truePath = getTruePath(path, _projectDirectory);
        auto meshReturn = Renderer::loadMesh(truePath);
        debugAssume(meshReturn.has_value(), "Failed to load mesh");

        Renderer::StaticMesh& mesh = *meshReturn;

        Renderer::MeshUploadOptions meshUploadOptions {
            .context = *_context,
            .commandBuffer = _queueCommandRepo->current(),
            .sceneBuffer = *_sceneBuffer,
        };

        Renderer::GPUStaticMesh gpuMesh = Renderer::uploadMesh(mesh, meshUploadOptions);
        if (_assetCache.hasMaterial(mesh.materialPath))
        {
            gpuMesh.material = _assetCache.getMaterial(mesh.materialPath);
        }

        _assetCache.addMesh(gpuMesh);
    }

    void Editor::closeLevel() noexcept
    {
        for (const auto& texturePath : _level->texturePaths)
        {
            _assetCache.clearTexture(texturePath);
        }

        for (const auto& materialPath : _level->materialPaths)
        {
            _assetCache.clearMaterial(materialPath);
        }

        for (const auto& meshPath : _level->meshPaths)
        {
            _assetCache.clearMesh(meshPath);
        }

        _level = {};
        _editorCameraEntity = entt::null;
    }

    void Editor::processModelImport(const ModelImportDetails& details) noexcept
    {
        std::filesystem::path truePath = getTruePath(details.modelPath, _projectDirectory);
        auto importResult = Renderer::importAsset2(truePath);

        if (!importResult.has_value())
        {
            return;
        }

        Renderer::AssetImportResult2& import = *importResult;

        // Save each texture
        std::string textureDirectory = appendFolder(details.outputDirectory, "textures");
        std::filesystem::create_directories(getTruePath(textureDirectory, _projectDirectory));

        std::unordered_map<size_t, std::string> textureIndices;

        for (size_t i = 0; i < importResult->textures.size(); i++)
        {
            auto& importTexturePath = importResult->textures[i];
            std::string outputTexturePath =
                textureDirectory + std::to_string(i).append(Renderer::TEXTURE_EXTENSION);
            std::filesystem::path trueTexturePath =
                getTruePath(outputTexturePath, _projectDirectory);

            auto textureReturn = Renderer::importTexture(importTexturePath);
            if (!textureReturn.has_value())
            {
                continue;
            }

            if (details.optimizeTexturePrecision)
            {
                Renderer::optimizePrecision(*textureReturn);
            }

            Renderer::Texture& texture = *textureReturn;
            texture.path = outputTexturePath;

            Renderer::AssetFile assetFile = Renderer::saveTexture(texture);
            auto saveResult = Renderer::saveAssetFile(trueTexturePath, assetFile);

            if (!saveResult.has_value())
            {
                continue;
            }

            // Add texture to project
            _project->texturePaths.insert(outputTexturePath);

            textureIndices[i] = outputTexturePath;
        }

        // Save each material
        std::string materialDirectory = appendFolder(details.outputDirectory, "materials");
        std::filesystem::create_directories(getTruePath(materialDirectory, _projectDirectory));

        std::unordered_map<size_t, std::string> materialIndices;

        for (size_t i = 0; i < importResult->materials.size(); i++)
        {
            auto& importMaterial = importResult->materials[i];
            std::string outputMaterialPath =
                materialDirectory + std::to_string(i).append(Renderer::MATERIAL_EXTENSION);
            std::filesystem::path trueMaterialPath =
                getTruePath(outputMaterialPath, _projectDirectory);

            Renderer::Material material {};
            material.path = outputMaterialPath;

            if (textureIndices.contains(importMaterial.albedoTextureIndex))
            {
                material.albedoTexturePath = textureIndices[importMaterial.albedoTextureIndex];
                material.albedoUseTexture = true;
            }

            if (textureIndices.contains(importMaterial.emissiveTextureIndex))
            {
                material.emissiveTexturePath = textureIndices[importMaterial.emissiveTextureIndex];
                material.emissiveUseTexture = true;
            }

            if (textureIndices.contains(importMaterial.normalTextureIndex))
            {
                material.normalTexturePath = textureIndices[importMaterial.normalTextureIndex];
                material.normalUseTexture = true;
            }

            if (textureIndices.contains(importMaterial.metallicTextureIndex))
            {
                material.metallicTexturePath = textureIndices[importMaterial.metallicTextureIndex];
                material.metallicUseTexture = true;
            }

            if (textureIndices.contains(importMaterial.roughnessTextureIndex))
            {
                material.roughnessTexturePath =
                    textureIndices[importMaterial.roughnessTextureIndex];
                material.roughnessUseTexture = true;
            }

            if (textureIndices.contains(importMaterial.aoTextureIndex))
            {
                material.occlusionTexturePath = textureIndices[importMaterial.aoTextureIndex];
                material.occlusionUseTexture = true;
            }

            Renderer::AssetFile assetFile = Renderer::saveMaterial(material);
            auto saveResult = Renderer::saveAssetFile(trueMaterialPath, assetFile);

            if (!saveResult.has_value())
            {
                continue;
            }

            // Add material to project
            _project->materialPaths.insert(outputMaterialPath);

            materialIndices[i] = outputMaterialPath;
        }

        // Save each mesh
        std::string meshDirectory = appendFolder(details.outputDirectory, "meshes");
        std::filesystem::create_directories(getTruePath(meshDirectory, _projectDirectory));

        std::unordered_map<size_t, std::string> meshIndices;

        for (size_t i = 0; i < importResult->meshes.size(); i++)
        {
            auto& importMesh = importResult->meshes[i];
            std::string outputMeshPath =
                meshDirectory + std::to_string(i).append(Renderer::MESH_EXTENSION);
            std::filesystem::path trueMeshPath = getTruePath(outputMeshPath, _projectDirectory);

            Renderer::StaticMesh mesh {};
            mesh.path = outputMeshPath;
            if (materialIndices.contains(importMesh.materialIndex))
            {
                mesh.materialPath = materialIndices[importMesh.materialIndex];
            }

            mesh.vertices = std::move(importMesh.vertices);
            mesh.indices = std::move(importMesh.indices);

            mesh.aabb = importMesh.aabb;

            mesh.lods.resize(1);
            mesh.lods[0].indexCount = static_cast<uint32_t>(mesh.indices.size());
            mesh.lods[0].indexOffset = 0;
            mesh.lods[0].vertexCount = static_cast<uint32_t>(mesh.vertices.size());
            mesh.lods[0].vertexOffset = 0;

            Renderer::AssetFile assetFile = Renderer::saveMesh(mesh);
            auto saveResult = Renderer::saveAssetFile(trueMeshPath, assetFile);

            if (!saveResult.has_value())
            {
                continue;
            }

            // Add mesh to project
            _project->meshPaths.insert(outputMeshPath);

            meshIndices[i] = outputMeshPath;
        }

        deduplicateProjectAssets();

        auto saveResult = Projects::saveProject(_projectPath, *_project);
        debugAssume(saveResult.has_value(), "Failed to save project");

        // import into level if selected
        if (!details.importIntoScene)
        {
            return;
        }

        // search to find necessary assets
        std::vector<std::string> texturePaths;
        std::vector<std::string> materialPaths;
        std::vector<std::string> meshPaths;

        for (size_t i = 0; i < importResult->nodes.size(); i++)
        {
            auto& node = importResult->nodes[i];

            if (meshIndices.contains(node.meshIndex))
            {
                std::string meshPath = meshIndices[node.meshIndex];

                auto& mesh = importResult->meshes[node.meshIndex];
                if (materialIndices.contains(mesh.materialIndex))
                {
                    std::string materialPath = materialIndices[mesh.materialIndex];
                    materialPaths.push_back(materialPath);

                    auto& material = importResult->materials[mesh.materialIndex];

                    auto eachTexture = [&](size_t textureIndex) -> void
                    {
                        if (textureIndices.contains(textureIndex))
                        {
                            std::string texturePath = textureIndices[textureIndex];
                            texturePaths.push_back(texturePath);
                        }
                    };

                    eachTexture(material.albedoTextureIndex);
                    eachTexture(material.emissiveTextureIndex);
                    eachTexture(material.normalTextureIndex);
                    eachTexture(material.metallicTextureIndex);
                    eachTexture(material.roughnessTextureIndex);
                    eachTexture(material.aoTextureIndex);
                }

                meshPaths.push_back(meshPath);
            }
        }

        // Add assets to level if they don't exist
        for (const auto& texturePath : texturePaths)
        {
            _level->texturePaths.insert(texturePath);
        }

        for (const auto& materialPath : materialPaths)
        {
            _level->materialPaths.insert(materialPath);
        }

        for (const auto& meshPath : meshPaths)
        {
            _level->meshPaths.insert(meshPath);
        }

        // Load textures
        for (const auto& texturePath : texturePaths)
        {
            loadTexture(texturePath);
        }

        for (const auto& materialPath : materialPaths)
        {
            loadMaterial(materialPath);
        }

        for (const auto& meshPath : meshPaths)
        {
            loadMesh(meshPath);
        }

        // Add nodes to scene
        auto& nodes = importResult->nodes;
        createChildrenImportedNodes(meshIndices, entt::null, importResult->rootNodes, nodes);
    }

    void Editor::createChildrenImportedNodes(
        const std::unordered_map<size_t, std::string>& meshIndices,
        exage::Entity parent,
        std::span<const size_t> children,
        const std::vector<Renderer::AssetImportResult2::Node>& nodes) noexcept
    {
        Scene& scene = _level->scene;

        for (const auto& child : children)
        {
            const auto& node = nodes[child];
            auto entity = scene.createEntity(parent);
            scene.addComponent<Transform3D>(entity, node.transform);

            std::string meshPath = meshIndices.at(node.meshIndex);
            if (_assetCache.hasMesh(meshPath))
            {
                const Renderer::GPUStaticMesh& mesh = _assetCache.getMesh(meshPath);
                Renderer::StaticMeshComponent meshComponent = {.path = mesh.path,
                                                               .pathHash = mesh.pathHash};
                scene.addComponent<Renderer::StaticMeshComponent>(entity, meshComponent);
            }

            createChildrenImportedNodes(meshIndices, entity, node.childrenIndices, nodes);
        }
    }

    void Editor::processTextureImport(const TextureImportDetails& details) noexcept
    {
        auto importResult = Renderer::importTexture(details.texturePath);
        if (!importResult.has_value())
        {
            return;
        }

        Renderer::Texture& texture = *importResult;

        if (details.optimizePrecision)
        {
            Renderer::optimizePrecision(texture);
        }

        Renderer::AssetFile assetFile = Renderer::saveTexture(texture);
        auto saveResult = Renderer::saveAssetFile(details.outputPath, assetFile);

        if (!saveResult.has_value())
        {
            return;
        }

        // Add texture to project
        _project->texturePaths.insert(details.outputPath);
    }

    void Editor::deduplicateProjectAssets() noexcept
    {
        // // Remove duplicate mesh paths
        // std::sort(_project->meshPaths.begin(), _project->meshPaths.end());
        // auto meshEnd = std::unique(_project->meshPaths.begin(), _project->meshPaths.end());
        // _project->meshPaths.erase(meshEnd, _project->meshPaths.end());

        // // Remove duplicate material paths
        // std::sort(_project->materialPaths.begin(), _project->materialPaths.end());
        // auto materialEnd =
        //     std::unique(_project->materialPaths.begin(), _project->materialPaths.end());
        // _project->materialPaths.erase(materialEnd, _project->materialPaths.end());

        // // Remove duplicate texture paths
        // std::sort(_project->texturePaths.begin(), _project->texturePaths.end());
        // auto textureEnd = std::unique(_project->texturePaths.begin(),
        // _project->texturePaths.end()); _project->texturePaths.erase(textureEnd,
        // _project->texturePaths.end());
    }
}  // namespace exitor
