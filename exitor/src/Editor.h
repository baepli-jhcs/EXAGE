#pragma once

#include "Panels/ComponentEditor.h"
#include "Panels/ComponentList.h"
#include "Panels/ContentBrowser.h"
#include "Panels/Hierarchy.h"
#include "Stages/AssetImport.h"
#include "Stages/ProjectSelector.h"
#include "Windows/TextureViewer.h"
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

    class Editor
    {
      public:
        Editor() noexcept;
        ~Editor();

        EXAGE_DELETE_COPY(Editor);
        EXAGE_DELETE_MOVE(Editor);

        void run() noexcept;

      private:
        void tick(float deltaTime) noexcept;

        void resizeCallback(glm::uvec2 extent) noexcept;

        std::unique_ptr<Window> _window;
        std::unique_ptr<Graphics::Context> _context;
        std::unique_ptr<Graphics::Swapchain> _swapchain;
        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;

        std::optional<exage::Graphics::QueueCommandRepo> _queueCommandRepo;
        std::optional<Graphics::ImGuiInstance> _imGui;
        std::optional<Renderer::FontManager> _fontManager;
        ImFont* _defaultFont = nullptr;

        Timer _timer;
    };
}  // namespace exitor
