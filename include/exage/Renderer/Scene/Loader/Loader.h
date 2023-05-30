#pragma once

#include <filesystem>
#include <optional>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Loader/Errors.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/SceneBuffer.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    [[nodiscard]] auto loadTexture(const std::filesystem::path& path) noexcept
        -> tl::expected<Texture, AssetLoadError>;

    [[nodiscard]] auto loadMaterial(const std::filesystem::path& path) noexcept
        -> tl::expected<Material, AssetLoadError>;

    [[nodiscard]] auto loadMesh(const std::filesystem::path& path) noexcept
        -> tl::expected<Mesh, AssetLoadError>;

    struct TextureUploadOptions
    {
        Graphics::Context& context;
        Graphics::CommandBuffer& commandBuffer;

        Graphics::Texture::Usage usage =
            Graphics::Texture::UsageFlags::eSampled | Graphics::Texture::UsageFlags::eTransferDst;
        Graphics::Texture::Layout layout = Graphics::Texture::Layout::eShaderReadOnly;
        Graphics::Access access = Graphics::AccessFlags::eShaderRead;
        Graphics::PipelineStage pipelineStage = Graphics::PipelineStageFlags::eFragmentShader;

        Graphics::SamplerCreateInfo samplerCreateInfo {};
    };

    struct MeshUploadOptions
    {
        Graphics::Context& context;
        Graphics::CommandBuffer& commandBuffer;
        SceneBuffer& sceneBuffer;

        Graphics::Access access = Graphics::AccessFlags::eShaderRead;
        Graphics::PipelineStage pipelineStage = Graphics::PipelineStageFlags::eFragmentShader;
    };

    [[nodiscard]] auto uploadTexture(const Texture& texture,
                                     const TextureUploadOptions& options) noexcept -> GPUTexture;

    [[nodiscard]] auto uploadMesh(const Mesh& mesh, const MeshUploadOptions& options) noexcept
        -> GPUMesh;

}  // namespace exage::Renderer
