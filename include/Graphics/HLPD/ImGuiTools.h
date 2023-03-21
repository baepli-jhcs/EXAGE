#pragma once

#include "Core/Core.h"
#include "Core/Window.h"
#include "Graphics/Context.h"
#include "imgui.h"

namespace exage::Graphics
{
    struct ImGuiInitInfo
    {
        Context& context;
        Queue& queue;
        Window& window;

        uint32_t maxImageCount = 3;
    };

    class ImGuiInstance
    {
      public:
        ImGuiInstance(const ImGuiInitInfo& initInfo) noexcept;
        ~ImGuiInstance();

        EXAGE_DELETE_COPY(ImGuiInstance);
        EXAGE_DEFAULT_MOVE(ImGuiInstance);

        void begin() noexcept;
        void end() noexcept;

        void addFont(const std::string& path, float size, bool isDefault = false) noexcept;

        void renderMainWindow(CommandBuffer& commandBuffer) noexcept;
        void renderAdditional();

        [[nodiscard]] auto getContext() const noexcept -> ImGuiContext* { return _imCtx; }

      private:
        void initGLFW(const ImGuiInitInfo& initInfo) noexcept;
        static void initVulkan(const ImGuiInitInfo& initInfo) noexcept;

        void buildFonts() noexcept;
        
        std::reference_wrapper<Context> _context;
        std::reference_wrapper<Queue> _queue;
        
        API _api;
        WindowAPI _windowAPI;

        ImGuiContext* _imCtx;
    };
}  // namespace exage::Graphics
