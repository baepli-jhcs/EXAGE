#include "TextureViewer.h"

#include <stdint.h>

#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "exage/Graphics/Sampler.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/Loader/Loader.h"
#include "exage/Renderer/Scene/Material.h"
#include "utils/files.h"

namespace exitor
{
    TextureViewer::TextureViewer(exage::Graphics::Context& context,
                                 exage::Renderer::AssetCache& assetCache) noexcept
        : _context(context)
        , _assetCache(assetCache)
    {
        exage::Graphics::SamplerCreateInfo const samplerCreateInfo {
            .anisotropy = exage::Graphics::Sampler::Anisotropy::e16,
            .filter = exage::Graphics::Sampler::Filter::eLinear,
            .mipmapMode = exage::Graphics::Sampler::MipmapMode::eLinear,
            .lodBias = 0.0F};

        _sampler = context.createSampler(samplerCreateInfo);

        exage::Graphics::TextureCreateInfo const textureCreateInfo {
            .extent = {1, 1, 1},
            .format = exage::Graphics::Format::eRGBA8,
            .type = exage::Graphics::Texture::Type::e2D,
            .usage = exage::Graphics::Texture::UsageFlags::eSampled
                | exage::Graphics::Texture::UsageFlags::eTransferDst,
            .arrayLayers = 1,
            .mipLevels = 1,
        };

        _defaultTexture = context.createTexture(textureCreateInfo);
    }

    void TextureViewer::draw(exage::Graphics::CommandBuffer& commandBuffer,
                             const exage::Projects::Project& project,
                             const std::filesystem::path& basePath) noexcept
    {
        if (!_defaultTextureLoaded)
        {
            std::shared_ptr<exage::Graphics::Buffer> stagingBuffer =
                _context.get().createBuffer(exage::Graphics::BufferCreateInfo {
                    .size = 4,
                    .mapMode = exage::Graphics::Buffer::MapMode::eMapped,
                    .cached = false});

            std::array<uint8_t, 4> data = {255, 255, 255, 255};
            std::span<const uint8_t> dataBytes = std::span(data);

            stagingBuffer->write(std::as_bytes(dataBytes), 0);

            commandBuffer.textureBarrier(_defaultTexture,
                                         exage::Graphics::Texture::Layout::eUndefined,
                                         exage::Graphics::Texture::Layout::eTransferDst,
                                         exage::Graphics::PipelineStageFlags::eTopOfPipe,
                                         exage::Graphics::PipelineStageFlags::eTransfer,
                                         exage::Graphics::Access {},
                                         exage::Graphics::AccessFlags::eTransferWrite);

            commandBuffer.copyBufferToTexture(
                stagingBuffer, _defaultTexture, 0, {0, 0, 0}, 0, 0, 1, {1, 1, 1});

            commandBuffer.textureBarrier(_defaultTexture,
                                         exage::Graphics::Texture::Layout::eTransferDst,
                                         exage::Graphics::Texture::Layout::eShaderReadOnly,
                                         exage::Graphics::PipelineStageFlags::eTransfer,
                                         exage::Graphics::PipelineStageFlags::eFragmentShader,
                                         exage::Graphics::AccessFlags::eTransferWrite,
                                         exage::Graphics::AccessFlags::eShaderRead);

            _defaultImGuiTexture = {
                _defaultTexture, _sampler, exage::Graphics::Texture::Aspect::eColor};

            _defaultTextureLoaded = true;
        }

        auto findTextureInProject = [&project](const std::string& textureName) -> bool
        {
            return std::ranges::any_of(project.texturePaths,
                                       [&textureName](const auto& texture)
                                       { return texture == textureName; });
        };

        if (!_showTextureViewer)
        {
            return;
        }

        if (!ImGui::Begin("Texture Viewer", &_showTextureViewer))
        {
            ImGui::End();
            return;
        }

        std::string currentTexture = _selectedTexture;

        // Texture selection
        if (ImGui::BeginCombo("Texture", _selectedTexture.c_str()))
        {
            for (const auto& texture : project.texturePaths)
            {
                bool isSelected = texture == _selectedTexture;

                if (ImGui::Selectable(texture.c_str(), isSelected))
                {
                    _selectedTexture = texture;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        if (currentTexture != _selectedTexture)
        {
            cleanupTexture(currentTexture);
        }

        ImGui::Separator();

        // get remaining space
        ImVec2 size = ImGui::GetContentRegionAvail();

        exage::Graphics::ImGuiTexture* imguiTexture = nullptr;
        ensureTextureLoaded(commandBuffer, basePath);
        if (!_selectedTexture.empty())
        {
            imguiTexture = &_imguiTextures[_selectedTexture];
        }
        else
        {
            imguiTexture = &_defaultImGuiTexture;
        }

        ImGui::Image(imguiTexture, size, {0, 1}, {1, 0});
    }

    void TextureViewer::ensureTextureLoaded(exage::Graphics::CommandBuffer& commandBuffer,
                                            const std::filesystem::path& basePath) noexcept
    {
        if (_selectedTexture.empty() || _textures.contains(_selectedTexture))
        {
            return;
        }

        if (_assetCache.get().hasTexture(_selectedTexture))
        {
            _textures[_selectedTexture] = _assetCache.get().getTexture(_selectedTexture);

            // Create ImGui texture
            _imguiTextures[_selectedTexture] = {_textures[_selectedTexture].texture,
                                                _sampler,
                                                exage::Graphics::Texture::Aspect::eColor};

            return;
        }

        // Load texture
        std::filesystem::path texturePath = getTruePath(_selectedTexture, basePath);
        auto loadResult = exage::Renderer::loadTexture(texturePath);

        if (!loadResult.has_value())
        {
            _selectedTexture.clear();
            return;
        }

        // Create GPU texture
        exage::Renderer::TextureUploadOptions uploadOptions {.context = _context.get(),
                                                             .commandBuffer = commandBuffer,
                                                             .useCompressedFormat = false};
        exage::Renderer::GPUTexture gpuTexture =
            exage::Renderer::uploadTexture(*loadResult, uploadOptions);

        _textures[_selectedTexture] = gpuTexture;

        _imguiTextures[_selectedTexture] = {
            gpuTexture.texture, _sampler, exage::Graphics::Texture::Aspect::eColor};
    }

    void TextureViewer::cleanupTexture(const std::string& path) noexcept
    {
        // Original plans were different, so this is a very unoptimized way of doing this.

        if (_textures.contains(path))
        {
            _textures.erase(path);
        }

        if (_imguiTextures.contains(path))
        {
            _imguiTextures.erase(path);
        }
    }
}  // namespace exitor