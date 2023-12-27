#pragma once

#include <filesystem>
#include <optional>
#include <unordered_set>

#include "exage/Core/Core.h"
#include "exage/Core/Errors.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Texture.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    [[nodiscard]] auto queryCompressedTextureSupport(Graphics::Context& context) noexcept
        -> std::unordered_set<Graphics::Format>;

    [[nodiscard]] auto loadTexture(const std::filesystem::path& path) noexcept
        -> tl::expected<Texture, Error>;

    [[nodiscard]] auto loadMaterial(const std::filesystem::path& path) noexcept
        -> tl::expected<Material, Error>;

    [[nodiscard]] auto loadMesh(const std::filesystem::path& path) noexcept
        -> tl::expected<StaticMesh, Error>;

    struct TextureUploadOptions
    {
        Graphics::Context& context;
        Graphics::CommandBuffer& commandBuffer;

        Graphics::Texture::Usage usage =
            Graphics::Texture::UsageFlags::eSampled | Graphics::Texture::UsageFlags::eTransferDst;
        Graphics::Texture::Layout layout = Graphics::Texture::Layout::eShaderReadOnly;
        Graphics::Access access = Graphics::AccessFlags::eShaderRead;
        Graphics::PipelineStage pipelineStage = Graphics::PipelineStageFlags::eFragmentShader;

        bool useCompressedFormat = true;
        std::unordered_set<Graphics::Format>* supportedCompressedFormats = nullptr;
        // If supportedCompressedFormats is nullptr, the function will query the supported formats
    };

    //    struct MeshUploadOptions
    //    {
    //        Graphics::Context& context;
    //        Graphics::CommandBuffer& commandBuffer;
    //
    //        Graphics::Access access = Graphics::AccessFlags::eShaderRead;
    //        Graphics::PipelineStage pipelineStage = Graphics::PipelineStageFlags::eFragmentShader;
    //    };

    [[nodiscard]] auto uploadTexture(const Texture& texture,
                                     const TextureUploadOptions& options) noexcept -> GPUTexture;

    //    [[nodiscard]] auto uploadMesh(const StaticMesh& mesh, const MeshUploadOptions& options)
    //    noexcept
    //        -> GPUStaticMesh;

}  // namespace exage::Renderer
