#pragma once

#include "Panels/ComponentEditor.h"
#include "Panels/ComponentList.h"
#include "Panels/ContentBrowser.h"
#include "Panels/Hierarchy.h"
#include "Stages/AssetImport.h"
#include "Stages/ProjectSelector.h"
#include "exage/Core/Core.h"
#include "exage/Core/Timer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "exage/Graphics/Queue.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Graphics/Utils/QueueCommand.h"
#include "exage/Projects/Level.h"
#include "exage/Projects/Project.h"
#include "exage/Renderer/Renderer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Loader/Converter.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Renderer/Utils/Fonts.h"
#include "exage/utils/classes.h"

namespace exitor
{
    using namespace exage;

    class Editor : public ContentBrowserCallbacks
    {
      public:
        Editor() noexcept;
        ~Editor() override;

        EXAGE_DELETE_COPY(Editor);
        EXAGE_DELETE_MOVE(Editor);

        void run() noexcept;

      private:
        void createDefaultAssets() noexcept;

        void prepareTestScene() noexcept;
        void createEditorCamera() noexcept;

        void tick(float deltaTime) noexcept;
        void drawGUI(float deltaTime) noexcept;

        void drawProjectSelector() noexcept;

        void resizeCallback(glm::uvec2 extent) noexcept;

        void loadTexture(const std::string& path) noexcept;
        void loadMaterial(const std::string& path) noexcept;
        void loadMesh(const std::string& path) noexcept;

        void loadLevelAssets() noexcept;
        void closeLevel() noexcept;

        void onComponentMeshSelection(const std::string& path) noexcept;

        // ContentBrowserCallbacks
        void recognizeMesh(const std::string& path) noexcept override {}
        void recognizeMaterial(const std::string& path) noexcept override {}
        void recognizeTexture(const std::string& path) noexcept override {}
        void recognizeLevel(const std::string& path) noexcept override {}
        void onMeshSelection(const std::string& path) noexcept override {}
        void onMaterialSelection(const std::string& path) noexcept override {}
        void onTextureSelection(const std::string& path) noexcept override {}
        void onLevelSelection(const std::string& path) noexcept override {}

        void processModelImport(const ModelImportDetails& details) noexcept;
        void createChildrenImportedNodes(
            const std::unordered_map<size_t, std::string>& meshIndices,
            exage::Entity parent,
            std::span<const size_t> children,
            const std::vector<Renderer::AssetImportResult2::Node>& node) noexcept;

        void deduplicateProjectAssets() noexcept;

        HierarchyPanel _hierarchyPanel;
        ComponentList _componentList;
        ComponentEditor _componentEditor;
        ContentBrowser _contentBrowser;
        AssetImport _assetImport;

        std::unique_ptr<Window> _window;
        std::unique_ptr<Graphics::Context> _context;
        std::unique_ptr<Graphics::Swapchain> _swapchain;
        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;

        std::optional<exage::Graphics::QueueCommandRepo> _queueCommandRepo;
        std::shared_ptr<Graphics::Sampler> _sampler;
        Graphics::ImGuiTexture _renderTexture;
        std::optional<Graphics::ImGuiInstance> _imGui;
        std::optional<Renderer::FontManager> _fontManager;

        ImFont* _defaultFont = nullptr;

        Renderer::GPUTexture _defaultTexture;
        Renderer::GPUMaterial _defaultMaterial;

        std::optional<Renderer::SceneBuffer> _sceneBuffer;
        std::optional<Renderer::Renderer> _renderer;

        std::optional<ProjectSelector> _projectSelector {};
        std::filesystem::path _projectPath;
        std::filesystem::path _projectDirectory;

        std::optional<Projects::Project> _project;
        std::optional<Projects::DeserializedLevel> _level;

        Entity _editorCameraEntity = entt::null;

        glm::uvec2 _viewportExtent = {600, 600};

        Renderer::AssetCache _assetCache;

        Timer _timer;
        glm::vec2 _lastMousePosition = {0.0f, 0.0f};

        Entity _selectedEntity = entt::null;
    };
}  // namespace exitor
