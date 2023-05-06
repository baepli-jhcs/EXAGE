#pragma once

#include "exage/Graphics/ResourceManager.h"

namespace exage::Graphics::RAII
{
    class BufferID
    {
      public:
        BufferID(ResourceManager& resourceManager, Buffer& buffer) noexcept
            : _resourceManager(resourceManager)
            , _id(_resourceManager.get().bindBuffer(buffer))
        {
        }
        ~BufferID() { _resourceManager.get().unbindBuffer(_id); }

        EXAGE_DELETE_COPY(BufferID);
        EXAGE_DEFAULT_MOVE(BufferID);

        [[nodiscard]] auto get() const noexcept -> exage::Graphics::BufferID { return _id; }
        [[nodiscard]] auto getResourceManager() const noexcept -> ResourceManager&
        {
            return _resourceManager;
        }

      private:
        std::reference_wrapper<ResourceManager> _resourceManager;
        exage::Graphics::BufferID _id;
    };

    class TextureID
    {
      public:
        TextureID(ResourceManager& resourceManager, Texture& texture) noexcept
            : _resourceManager(resourceManager)
            , _id(_resourceManager.get().bindTexture(texture))
        {
        }
        ~TextureID() { _resourceManager.get().unbindTexture(_id); }

        EXAGE_DELETE_COPY(TextureID);
        EXAGE_DEFAULT_MOVE(TextureID);

        [[nodiscard]] auto get() const noexcept -> exage::Graphics::TextureID { return _id; }
        [[nodiscard]] auto getResourceManager() const noexcept -> ResourceManager&
        {
            return _resourceManager;
        }

      private:
        std::reference_wrapper<ResourceManager> _resourceManager;
        exage::Graphics::TextureID _id;
    };

}  // namespace exage::Graphics::RAII
