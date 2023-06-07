#pragma once

#include "exage/Core/Core.h"
#include "exage/Core/Window.h"
#include "exage/Graphics/Context.h"
#include "imgui.h"

namespace exage::Graphics
{
    struct ImGuiInitInfo
    {
        Context& context;
        Window& window;
    };

    class ImGuiInstance
    {
      public:
        explicit ImGuiInstance(const ImGuiInitInfo& initInfo) noexcept;
        ~ImGuiInstance();

        EXAGE_DELETE_COPY(ImGuiInstance);
        ImGuiInstance(ImGuiInstance&& old) noexcept;
        auto operator=(ImGuiInstance&& old) noexcept -> ImGuiInstance&;

        void begin() noexcept;
        void end() noexcept;

        void addFont(const std::string& path, float size, bool isDefault = false) noexcept;

        void processEvent(const Event& event) noexcept;

        void renderMainWindow(CommandBuffer& commandBuffer) noexcept;
        void renderAdditional();

        [[nodiscard]] auto getContext() const noexcept -> ImGuiContext* { return _imCtx; }

        void buildFonts() noexcept;

      private:
        void cleanup() noexcept;

        void initGLFW(const ImGuiInitInfo& initInfo) noexcept;
        static void initVulkan(const ImGuiInitInfo& initInfo) noexcept;

        std::reference_wrapper<Context> _context;

        API _api;
        WindowAPI _windowAPI;

        ImGuiContext* _imCtx;
    };
}  // namespace exage::Graphics
