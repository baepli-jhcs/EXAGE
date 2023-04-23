#pragma once

#include <filesystem>
#include <optional>

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Renderer/Scene/AssetCache.h"
#include "exage/Renderer/Scene/Material.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/Loader/Errors.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct AssetImportOptions
    {
        std::filesystem::path assetPath;
        std::filesystem::path saveDirectory;
        bool overwriteExisting = false;

        Scene* scene = nullptr;
        AssetCache* cache = nullptr;
        Entity parent = entt::null;

        Graphics::Context* context = nullptr;
        Graphics::CommandBuffer* commandBuffer = nullptr;
        Graphics::ResourceManager* resourceManager = nullptr;
        Graphics::Texture::Layout layout = Graphics::Texture::Layout::eShaderReadOnly;
        Graphics::Access access = Graphics::AccessFlags::eShaderRead;
        Graphics::PipelineStage pipelineStage = Graphics::PipelineStageFlags::eFragmentShader;
    };

    [[nodiscard]] EXAGE_EXPORT auto importAsset(const AssetImportOptions& options) noexcept
        -> std::optional<AssetImportError>;

    struct TextureImportOptions
    {
        std::filesystem::path texturePath;
        std::filesystem::path savePath;
        bool overwriteExisting = false;

        Graphics::Context* context = nullptr;
        Graphics::CommandBuffer* commandBuffer = nullptr;
        Graphics::ResourceManager* resourceManager = nullptr;
        Graphics::Texture::Layout layout = Graphics::Texture::Layout::eShaderReadOnly;
        Graphics::Access access = Graphics::AccessFlags::eShaderRead;
        Graphics::PipelineStage pipelineStage = Graphics::PipelineStageFlags::eFragmentShader;
    };

    [[nodiscard]] EXAGE_EXPORT auto importTexture(const TextureImportOptions& options) noexcept
        -> std::shared_ptr<Texture>;

}  // namespace exage::Renderer
