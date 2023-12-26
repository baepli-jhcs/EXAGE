#include "LevelPanel.h"

#include <exage/Graphics/Queue.h>

#include "exage/Graphics/Sampler.h"
#include "imgui.h"

namespace exitor
{
    constexpr static auto DEFAULT_LEVEL_PANEL_SIZE = glm::uvec2(800, 800);

    LevelPanel::LevelPanel(Graphics::Context& context, Projects::Level& level) noexcept
        : _context(&context)
        , _level(&level)
        , _viewportExtent(DEFAULT_LEVEL_PANEL_SIZE)
    {
        Graphics::TextureCreateInfo textureCreateInfo {.extent = {_viewportExtent, 1}};
        _testTexture = _context->createTexture(textureCreateInfo);

        Graphics::BufferCreateInfo bufferCreateInfo {};
        bufferCreateInfo.size = _viewportExtent.x * _viewportExtent.y * 4;
        bufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;

        std::vector<uint8_t> data(bufferCreateInfo.size);
        for (size_t i = 0; i < bufferCreateInfo.size; i += 4)
        {
            data[i] = 255;
            data[i + 1] = 0;
            data[i + 2] = 0;
            data[i + 3] = 255;
        }
        std::span<const std::byte> const bytes = std::as_bytes(std::span(data));
        auto buffer = _context->createBuffer(bufferCreateInfo);
        buffer->write(bytes, 0);

        auto commandBuffer = _context->createCommandBuffer();
        commandBuffer->begin();

        commandBuffer->textureBarrier(_testTexture,
                                      Graphics::Texture::Layout::eUndefined,
                                      Graphics::Texture::Layout::eTransferDst,
                                      Graphics::PipelineStage {},
                                      Graphics::PipelineStageFlags::eTransfer,
                                      Graphics::AccessFlags {},
                                      Graphics::AccessFlags::eTransferWrite,
                                      Graphics::QueueOwnership::eGraphics,
                                      Graphics::QueueOwnership::eGraphics);

        commandBuffer->copyBufferToTexture(
            buffer, _testTexture, 0, glm::uvec3 {0}, 0, 0, 1, {_viewportExtent, 1});

        commandBuffer->textureBarrier(_testTexture,
                                      Graphics::Texture::Layout::eTransferDst,
                                      Graphics::Texture::Layout::eShaderReadOnly,
                                      Graphics::PipelineStageFlags::eTransfer,
                                      Graphics::PipelineStageFlags::eFragmentShader,
                                      Graphics::AccessFlags::eTransferWrite,
                                      Graphics::AccessFlags::eShaderRead,
                                      Graphics::QueueOwnership::eGraphics,
                                      Graphics::QueueOwnership::eGraphics);
        commandBuffer->end();

        _context->getQueue().submitTemporary(std::move(commandBuffer));
        // TODO: remove everything above

        Graphics::SamplerCreateInfo samplerCreateInfo {};
        samplerCreateInfo.anisotropy = Graphics::Sampler::Anisotropy::e16;
        samplerCreateInfo.filter = Graphics::Sampler::Filter::eLinear;
        samplerCreateInfo.mipmapMode = Graphics::Sampler::MipmapMode::eLinear;

        _sampler = _context->createSampler(samplerCreateInfo);

        _imTexture.sampler = _sampler;
        _imTexture.aspect = Graphics::Texture::Aspect::eColor;
        _imTexture.texture = _testTexture;
    }

    void LevelPanel::setLevel(Projects::Level& level) noexcept
    {
        _level = &level;
    }

    void LevelPanel::run(Graphics::CommandBuffer& commandBuffer, float deltaTime) noexcept
    {
        std::string levelName = _level->path.empty() ? "*Untitled*" : _level->path;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin(levelName.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

        ImVec2 viewportWindowSize = ImGui::GetContentRegionAvail();
        if (viewportWindowSize.x < 0.0F)
        {
            viewportWindowSize.x = 0.0F;
        }
        if (viewportWindowSize.y < 0.0F)
        {
            viewportWindowSize.y = 0.0F;
        }

        _viewportExtent = glm::uvec2(viewportWindowSize.x, viewportWindowSize.y);

        ImGui::Image(&_imTexture, viewportWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();
    }
}  // namespace exitor