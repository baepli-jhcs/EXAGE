﻿#pragma once

#include "LevelEditor/LevelEditor.h"
#include "ProjectSelector/ProjectSelector.h"
#include "exage/Core/Core.h"
#include "exage/Core/Timer.h"
#include "exage/GUI/Fonts.h"
#include "exage/GUI/ImGui.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/FrameBuffer.h"
#include "exage/Graphics/Queue.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Graphics/Utils/QueueCommand.h"
#include "exage/Projects/Level.h"
#include "exage/Projects/Project.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Loader/Converter.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/utils/classes.h"
#include "imgui.h"

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
        void handleFonts() noexcept;
        void tickGUI(float deltaTime) noexcept;

        std::unique_ptr<System::Window> _window;
        std::unique_ptr<Graphics::Context> _context;
        std::unique_ptr<Graphics::Swapchain> _swapchain;
        std::shared_ptr<Graphics::FrameBuffer> _frameBuffer;

        std::optional<exage::Graphics::QueueCommandRepo> _queueCommandRepo;
        std::optional<GUI::ImGui::Instance> _imGui;
        std::optional<GUI::ImGui::FontManager> _fontManager;
        ImFont* _defaultFont = nullptr;

        std::optional<ProjectSelector> _projectSelector;
        std::optional<LevelEditor> _levelEditor;

        Timer _timer;
    };
}  // namespace exitor
