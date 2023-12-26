#include <fstream>

#include "exage/GUI/Fonts.h"

#include "imgui.h"

namespace exage::GUI
{

    auto ImGui::FontManager::addFont(const std::filesystem::path& path,
                                     const std::string& name) noexcept -> bool
    {
        if (_fonts.contains(name))
        {
            return false;
        }

        // Load font as std::vector<std::byte>
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        std::size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<std::byte> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);

        _fontData[name] = std::move(data);

        auto& fontMap = _fonts[name];

        ImGuiIO& io = ::ImGui::GetIO();
        ImFont* font =
            io.Fonts->AddFontFromMemoryTTF(_fontData[name].data(), _fontData[name].size(), 12.0F);
        fontMap[12] = font;

        _rebuildFonts = true;

        return true;
    }

    auto ImGui::FontManager::getFont(const std::string& name, uint32_t size) noexcept -> ImFont*
    {
        if (!_fonts.contains(name))
        {
            return nullptr;
        }

        auto& fontMap = _fonts[name];

        if (!fontMap.contains(size))
        {
            ImGuiIO& io = ::ImGui::GetIO();
            ImFont* font = io.Fonts->AddFontFromMemoryTTF(_fontData[name].data(),
                                                          static_cast<int>(_fontData[name].size()),
                                                          static_cast<float>(size));
            fontMap[size] = font;

            _rebuildFonts = true;
        }

        return fontMap[size];
    }

    void ImGui::FontManager::newFrame() noexcept
    {
        if (_rebuildFonts)
        {
            _imGuiInstance->buildFonts();
            _rebuildFonts = false;
        }
    }

}  // namespace exage::GUI