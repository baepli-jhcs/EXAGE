#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Texture.h"
#include "exage/System/Window.h"

class ImGuiContext;

namespace exage::GUI::ImGui
{
    struct InitInfo
    {
        Graphics::Context& context;
        System::Window& window;
    };
    class Instance
    {
      public:
        explicit Instance(const InitInfo& initInfo) noexcept;
        ~Instance();

        EXAGE_DELETE_COPY(Instance);
        Instance(Instance&& old) noexcept;
        auto operator=(Instance&& old) noexcept -> Instance&;

        void begin() noexcept;
        void end() noexcept;

        void addFont(const std::string& path, float size, bool isDefault = false) noexcept;

        void processEvent(const System::Event& event) noexcept;

        void renderMainWindow(Graphics::CommandBuffer& commandBuffer) noexcept;
        void renderAdditional();

        [[nodiscard]] auto getContext() const noexcept -> ImGuiContext* { return _imCtx; }

        void buildFonts() noexcept;

      private:
        void cleanup() noexcept;

        void initGLFW(const InitInfo& initInfo) noexcept;
        static void initVulkan(const InitInfo& initInfo) noexcept;

        std::reference_wrapper<Graphics::Context> _context;

        Graphics::API _api;
        System::API _windowAPI;

        ImGuiContext* _imCtx;
    };
    struct Texture
    {
        std::shared_ptr<Graphics::Texture> texture;
        std::shared_ptr<Graphics::Sampler> sampler;
        Graphics::Texture::Aspect aspect = Graphics::Texture::Aspect::eColor;
    };
}  // namespace exage::GUI::ImGui
