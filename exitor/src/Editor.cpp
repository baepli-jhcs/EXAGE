#include <algorithm>

#include "Editor.h"

#include <fmt/format.h>
#include <glm/trigonometric.hpp>

#include "exage/Core/Debug.h"
#include "exage/Core/Event.h"
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
#include "exage/utils/math.h"
#include "imgui.h"

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

        _fontManager = Renderer::FontManager {*_imGui};

        _fontManager->addFont("assets/exage/fonts/SourceSansPro/Regular.ttf",
                              "Source Sans Pro Regular");
        _fontManager->addFont("assets/exage/fonts/SourceSansPro/Bold.ttf", "Source Sans Pro Bold");

        Renderer::SceneBufferCreateInfo sceneBufferCreateInfo {.context = *_context};
        _sceneBuffer = Renderer::SceneBuffer {sceneBufferCreateInfo};

        Renderer::RendererCreateInfo rendererCreateInfo {.context = *_context,
                                                         .sceneBuffer = *_sceneBuffer,
                                                         .assetCache = _assetCache,
                                                         .extent = _viewportExtent};
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
        transform.rotation = Rotation3D {{0, 0, 0}, RotationType::ePitchYawRoll};

        Renderer::setSceneCamera(_scene, cameraEntity);

        auto importResult = Renderer::importAsset2("assets/exage/models/sponza/Sponza.gltf");
        debugAssume(importResult.has_value(), "Failed to import asset");

        std::vector<Renderer::GPUTexture> gpuTextures;
        gpuTextures.reserve(importResult->textures.size());

        auto supportedFormats = Renderer::queryCompressedTextureSupport(*_context);

        auto commamdBuffer = _context->createCommandBuffer();
        commamdBuffer->begin();

        Renderer::TextureUploadOptions uploadOptions {.context = *_context,
                                                      .commandBuffer = *commamdBuffer};
        uploadOptions.supportedCompressedFormats = &supportedFormats;
        uploadOptions.useCompressedFormat = false;

        uploadOptions.samplerCreateInfo = {
            .anisotropy = Graphics::Sampler::Anisotropy::e16,
            .filter = Graphics::Sampler::Filter::eLinear,
            .mipmapMode = Graphics::Sampler::MipmapMode::eLinear,
        };

        for (size_t i = 0; i < importResult->textures.size(); i++)
        {
            auto& texturePath = importResult->textures[i];
            std::filesystem::path texturePath2 =
                std::filesystem::path("assets/exage/models/exspon/main/textures/")
                / (std::to_string(i).append(Renderer::TEXTURE_EXTENSION));
            auto textureReturn = Renderer::importTexture(texturePath);
            if (textureReturn.has_value())
            {
                Renderer::Texture& texture = *textureReturn;
                auto saveResult = Renderer::saveTexture(texture, texturePath2, "");
                debugAssume(saveResult.has_value(), "Failed to save texture");

                Renderer::GPUTexture gpuTexture = Renderer::uploadTexture(texture, uploadOptions);
                gpuTextures.push_back(gpuTexture);

                _assetCache.addTexture(gpuTexture);
            }
            else
            {
                gpuTextures.push_back(Renderer::GPUTexture {});
            }
        }

        std::vector<Renderer::GPUMaterial> gpuMaterials;
        gpuMaterials.reserve(importResult->materials.size());

        for (size_t i = 0; i < importResult->materials.size(); i++)
        {
            auto& material = importResult->materials[i];

            Renderer::Material material2 {};
            material2.albedoColor = material.albedoColor;
            material2.emissiveColor = material.emissiveColor;
            material2.metallicValue = material.metallicValue;
            material2.roughnessValue = material.roughnessValue;

            if (material.albedoTextureIndex < gpuTextures.size())
            {
                if (gpuTextures[material.albedoTextureIndex].texture)
                {
                    material2.albedoTexturePath = gpuTextures[material.albedoTextureIndex].path;
                    material2.albedoUseTexture = true;
                }
            }

            // Normal
            if (material.normalTextureIndex < gpuTextures.size())
            {
                if (gpuTextures[material.normalTextureIndex].texture)
                {
                    material2.normalTexturePath = gpuTextures[material.normalTextureIndex].path;
                    material2.normalUseTexture = true;
                }
            }

            if (material.metallicTextureIndex < gpuTextures.size())
            {
                if (gpuTextures[material.metallicTextureIndex].texture)
                {
                    material2.metallicTexturePath = gpuTextures[material.metallicTextureIndex].path;
                    material2.metallicUseTexture = true;
                }
            }

            if (material.roughnessTextureIndex < gpuTextures.size())
            {
                if (gpuTextures[material.roughnessTextureIndex].texture)
                {
                    material2.roughnessTexturePath =
                        gpuTextures[material.roughnessTextureIndex].path;
                    material2.roughnessUseTexture = true;
                }
            }

            if (material.aoTextureIndex < gpuTextures.size())
            {
                if (gpuTextures[material.aoTextureIndex].texture)
                {
                    material2.occlusionTexturePath = gpuTextures[material.aoTextureIndex].path;
                    material2.occlusionUseTexture = true;
                }
            }

            if (material.emissiveTextureIndex < gpuTextures.size())
            {
                if (gpuTextures[material.emissiveTextureIndex].texture)
                {
                    material2.emissiveTexturePath = gpuTextures[material.emissiveTextureIndex].path;
                    material2.emissiveUseTexture = true;
                }
            }

            std::filesystem::path materialPath =
                std::filesystem::path("assets/exage/models/exspon/main/materials/")
                / (std::to_string(i).append(Renderer::MATERIAL_EXTENSION));

            auto saveResult = Renderer::saveMaterial(material2, materialPath, "");

            debugAssume(saveResult.has_value(), "Failed to save material");

            Renderer::GPUMaterial gpuMaterial {.path = material2.path};
            if (_assetCache.hasTexture(material2.albedoTexturePath))
            {
                gpuMaterial.albedoTexture = _assetCache.getTexture(material2.albedoTexturePath);
            }
            if (_assetCache.hasTexture(material2.emissiveTexturePath))
            {
                gpuMaterial.emissiveTexture = _assetCache.getTexture(material2.emissiveTexturePath);
            }
            if (_assetCache.hasTexture(material2.normalTexturePath))
            {
                gpuMaterial.normalTexture = _assetCache.getTexture(material2.normalTexturePath);
            }
            if (_assetCache.hasTexture(material2.metallicTexturePath))
            {
                gpuMaterial.metallicTexture = _assetCache.getTexture(material2.metallicTexturePath);
            }
            if (_assetCache.hasTexture(material2.roughnessTexturePath))
            {
                gpuMaterial.roughnessTexture =
                    _assetCache.getTexture(material2.roughnessTexturePath);
            }
            if (_assetCache.hasTexture(material2.occlusionTexturePath))
            {
                gpuMaterial.occlusionTexture =
                    _assetCache.getTexture(material2.occlusionTexturePath);
            }

            Renderer::GPUMaterial::Data data =
                Renderer::materialDataFromGPUAndCPU(gpuMaterial, material2);

            Graphics::BufferCreateInfo bufferCreateInfo {
                .size = sizeof(Renderer::GPUMaterial::Data),
                .mapMode = Graphics::Buffer::MapMode::eMapped,
                .cached = false,
            };

            gpuMaterial.buffer = _context->createBuffer(bufferCreateInfo);
            gpuMaterial.buffer->write(std::as_bytes(std::span(&data, 1)), 0);

            _assetCache.addMaterial(gpuMaterial);
            gpuMaterials.push_back(gpuMaterial);

            material = {};  // Memory cleanup
        }

        std::vector<Renderer::GPUMesh> gpuMeshes;
        gpuMeshes.reserve(importResult->meshes.size());

        Renderer::MeshUploadOptions meshUploadOptions {
            .context = *_context,
            .commandBuffer = *commamdBuffer,
            .sceneBuffer = *_sceneBuffer,
        };

        for (size_t i = 0; i < importResult->meshes.size(); i++)
        {
            auto& mesh = importResult->meshes[i];

            Renderer::Mesh mesh2 {};
            mesh2.vertices = std::move(mesh.vertices);
            mesh2.indices = std::move(mesh.indices);
            mesh2.aabb = mesh.aabb;
            mesh2.materialPath = gpuMaterials[mesh.materialIndex].path;
            mesh2.lods.resize(1);

            mesh2.lods[0].indexCount = static_cast<uint32_t>(mesh2.indices.size());
            mesh2.lods[0].indexOffset = 0;
            mesh2.lods[0].vertexCount = static_cast<uint32_t>(mesh2.vertices.size());
            mesh2.lods[0].vertexOffset = 0;

            std::filesystem::path meshPath =
                std::filesystem::path("assets/exage/models/exspon/main/meshes/")
                / (std::to_string(i).append(Renderer::MESH_EXTENSION));

            auto saveResult = Renderer::saveMesh(mesh2, meshPath, "");
            debugAssume(saveResult.has_value(), "Failed to save mesh");

            Renderer::GPUMesh gpuMesh = Renderer::uploadMesh(mesh2, meshUploadOptions);

            if (_assetCache.hasMaterial(mesh2.materialPath))
            {
                gpuMesh.material = _assetCache.getMaterial(mesh2.materialPath);
            }

            _assetCache.addMesh(gpuMesh);
            gpuMeshes.push_back(gpuMesh);

            mesh = {};  // Memory cleanup
        }

        commamdBuffer->end();
        _context->getQueue().submitTemporary(std::move(commamdBuffer));

        Renderer::AssetSceneImportInfo sceneImportInfo {.meshes = gpuMeshes,
                                                        .rootNodes = importResult->rootNodes,
                                                        .nodes = importResult->nodes};
        Renderer::importScene(sceneImportInfo, _scene);
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

            drawGUI(deltaTime);

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
        _swapchain->resize(extent);
    }

    void Editor::drawGUI(float deltaTime) noexcept
    {
        ImFont* font = _fontManager->getFont("Source Sans Pro Regular", 16.F);

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

        ImGui::PushFont(font);

        ImGui::Begin("DockSpace", &open, windowFlags);

        if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0)
        {
            ImGuiID const dockspaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), dockspaceFlags);
        }

        bool showViewport = true;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport", &showViewport, ImGuiWindowFlags_AlwaysAutoResize);

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
                auto camera = Renderer::getSceneCamera(_scene);
                auto& transform = _scene.getComponent<Transform3D>(camera);

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
            }
        }

        ImGui::Image(_renderer->getFrameBuffer().getTexture(0).get(), viewportWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();

        if (viewportSize != _renderer->getExtent())
        {
            _renderer->resize(viewportSize);
        }

        ImGui::Begin("Render Info");
        ImGui::Text("FPS: %f", 1.0F / deltaTime);
        ImGui::Text("Frame time: %f", deltaTime);
        ImGui::End();

        exage::Entity selectedEntity =
            _hierarchyPanel.draw(_scene, Renderer::getSceneCamera(_scene));
        entt::id_type type = _componentList.draw(_scene, selectedEntity);
        _componentEditor.draw(_scene, selectedEntity, type);

        ImGui::End();

        ImGui::PopFont();

        _lastMousePosition = currentMousePos;
    }
}  // namespace exitor
