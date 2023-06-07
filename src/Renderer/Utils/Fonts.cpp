#include <fstream>

#include "exage/Renderer/Utils/Fonts.h"

namespace exage::Renderer
{

    auto FontManager::addFont(const std::filesystem::path& path, const std::string& name) noexcept
        -> bool
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

        ImGuiIO& io = ImGui::GetIO();
        ImFont* font =
            io.Fonts->AddFontFromMemoryTTF(_fontData[name].data(), _fontData[name].size(), 12.0f);
        fontMap[12.0f] = font;

        if (_imGuiInstance != nullptr)
        {
            _imGuiInstance->buildFonts();
        }

        return true;
    }

    auto FontManager::getFont(const std::string& name, float size) noexcept -> ImFont*
    {
        if (!_fonts.contains(name))
        {
            return nullptr;
        }

        auto& fontMap = _fonts[name];

        if (!fontMap.contains(size))
        {
            ImGuiIO& io = ImGui::GetIO();
            ImFont* font = io.Fonts->AddFontFromMemoryTTF(
                _fontData[name].data(), static_cast<int>(_fontData[name].size()), size);
            fontMap[size] = font;

            if (_imGuiInstance != nullptr)
            {
                _imGuiInstance->buildFonts();
            }
        }

        return fontMap[size];
    }

}  // namespace exage::Renderer