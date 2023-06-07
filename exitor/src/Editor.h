﻿#pragma once

#include "Panels/ComponentList.h"
#include "Panels/Hierarchy.h"
#include "exage/Core/Core.h"
#include "exage/Core/Timer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "exage/Graphics/Queue.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Graphics/Utils/QueueCommand.h"
#include "exage/Renderer/Renderer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/SceneBuffer.h"

namespace exitor
{
    using namespace exage;

    class Editor
    {
      public:
        Editor() noexcept;
        ~Editor();

        void run() noexcept;

      private:
        void prepareTestScene() noexcept;

        void resizeCallback(glm::uvec2 extent);

        void drawGUI(float deltaTime) noexcept;

        HierarchyPanel _hierarchyPanel;
        ComponentList _componentList;

        std::unique_ptr<Window> _window;
        std::unique_ptr<Graphics::Context> _context;
        std::unique_ptr<Graphics::Swapchain> _swapchain;
        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;

        std::optional<exage::Graphics::QueueCommandRepo> _queueCommandRepo;
        std::optional<Graphics::ImGuiInstance> _imGui;

        std::optional<Renderer::SceneBuffer> _sceneBuffer;
        std::optional<Renderer::Renderer> _renderer;

        glm::uvec2 _viewportExtent = {600, 600};

        Scene _scene;

        Renderer::AssetCache _assetCache;

        Timer _timer;
        glm::vec2 _lastMousePosition = {0.0f, 0.0f};
    };
}  // namespace exitor
