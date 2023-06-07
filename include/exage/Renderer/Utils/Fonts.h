#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "imgui.h"

namespace exage::Renderer
{
    class FontManager
    {
      public:
        FontManager() noexcept = default;
        explicit FontManager(Graphics::ImGuiInstance& imGuiInstance) noexcept
            : _imGuiInstance(&imGuiInstance)
        {
        }
        ~FontManager() = default;

        EXAGE_DELETE_COPY(FontManager);
        EXAGE_DEFAULT_MOVE(FontManager);

        auto addFont(const std::filesystem::path& path, const std::string& name) noexcept -> bool;
        [[nodiscard]] auto getFont(const std::string& name, float size) noexcept -> ImFont*;

      private:
        Graphics::ImGuiInstance* _imGuiInstance = nullptr;

        std::unordered_map<std::string, std::unordered_map<float, ImFont*>> _fonts;
        std::unordered_map<std::string, std::vector<std::byte>> _fontData;
    };
}  // namespace exage::Renderer