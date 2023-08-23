#pragma once

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "exage/Graphics/Sampler.h"
#include "exage/Projects/Project.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Material.h"

namespace exitor
{
    class TextureViewer
    {
      public:
        explicit TextureViewer(exage::Graphics::Context& context,
                               exage::Renderer::AssetCache& assetCache) noexcept;
        ~TextureViewer() noexcept = default;

        EXAGE_DELETE_COPY(TextureViewer);
        EXAGE_DELETE_MOVE(TextureViewer);

        void draw(exage::Graphics::CommandBuffer& commandBuffer,
                  const exage::Projects::Project& project,
                  const std::filesystem::path& basePath) noexcept;

        void setOpened(bool opened) noexcept { _showTextureViewer = opened; }
        [[nodiscard]] auto isOpened() const noexcept -> bool { return _showTextureViewer; }

      private:
        void ensureTextureLoaded(exage::Graphics::CommandBuffer& commandBuffer,
                                 const std::filesystem::path& basePath) noexcept;
        void cleanupTexture(const std::string& path) noexcept;

        std::reference_wrapper<exage::Graphics::Context> _context;
        std::reference_wrapper<exage::Renderer::AssetCache> _assetCache;
        std::shared_ptr<exage::Graphics::Sampler> _sampler;
        std::shared_ptr<exage::Graphics::Texture> _defaultTexture;
        exage::Graphics::ImGuiTexture _defaultImGuiTexture;
        bool _defaultTextureLoaded = false;

        bool _showTextureViewer = false;
        std::string _selectedTexture;

        std::unordered_map<std::string, exage::Renderer::GPUTexture> _textures;
        std::unordered_map<std::string, exage::Graphics::ImGuiTexture> _imguiTextures;
    };
}  // namespace exitor