#pragma once

#include "exage/Core/Core.h"
#include "exage/GUI/ImGui.h"

struct ImFont;

namespace exage::GUI::ImGui
{
    class FontManager
    {
      public:
        FontManager() noexcept = default;
        explicit FontManager(Instance& imGuiInstance) noexcept
            : _imGuiInstance(&imGuiInstance)
        {
        }
        ~FontManager() = default;

        EXAGE_DELETE_COPY(FontManager);
        EXAGE_DEFAULT_MOVE(FontManager);

        void newFrame() noexcept;

        auto addFont(const std::filesystem::path& path, const std::string& name) noexcept -> bool;
        [[nodiscard]] auto getFont(const std::string& name, uint32_t size) noexcept -> ImFont*;

      private:
        Instance* _imGuiInstance = nullptr;
        bool _rebuildFonts = false;

        std::unordered_map<std::string, std::unordered_map<uint32_t, ImFont*>> _fonts;
        std::unordered_map<std::string, std::vector<std::byte>> _fontData;
    };
}  // namespace exage::GUI::ImGui
