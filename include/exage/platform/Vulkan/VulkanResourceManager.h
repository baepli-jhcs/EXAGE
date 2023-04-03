#pragma once

#include "exage/Graphics/ResourceManager.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    namespace detail
    {
        class VulkanResourcePool
        {
          public:
            VulkanResourcePool(uint32_t maxResourceCount) noexcept
            {
                freeSlots.resize(maxResourceCount);
                for (uint32_t i = 0; i < maxResourceCount; ++i)
                {
                    freeSlots[i] = i;
                }
            }

            [[nodiscard]] auto allocate() noexcept -> uint32_t
            {
                if (freeSlots.empty())
                {
                    return std::numeric_limits<uint32_t>::max();
                }

                std::lock_guard lock {mutex};

                const uint32_t slot = freeSlots.back();
                freeSlots.pop_back();
                return slot;
            }

            void free(uint32_t slot) noexcept
            {
                std::lock_guard lock {mutex};

                freeSlots.push_back(slot);
            }

          private:
            std::vector<uint32_t> freeSlots;
            std::mutex mutex;
        };
    }  // namespace detail

    class EXAGE_EXPORT VulkanResourceManager final : public ResourceManager
    {
      public:
        using ResourcePool = detail::VulkanResourcePool;

        constexpr static uint32_t maxBufferCount = 1048576;
        constexpr static uint32_t maxTextureCount = 1048576;

        constexpr static uint32_t sampledTextureBinding = 0;
        constexpr static uint32_t storageBufferBinding = 1;
        constexpr static uint32_t storageTextureBinding = 2;

        VulkanResourceManager(VulkanContext& context) noexcept;
        ~VulkanResourceManager() override;

        EXAGE_DELETE_COPY(VulkanResourceManager);
        EXAGE_DELETE_MOVE(VulkanResourceManager);

        [[nodiscard]] auto bindBuffer(Buffer& buffer) noexcept -> BufferID override;
        [[nodiscard]] auto bindTexture(Texture& texture) noexcept -> TextureID override;

        void unbindBuffer(BufferID buffer) noexcept override;
        void unbindTexture(TextureID texture) noexcept override;

        [[nodiscard]] auto getDescriptorSet() const noexcept -> vk::DescriptorSet
        {
            return _descriptorSet;
        }
        [[nodiscard]] auto getDescriptorSetLayout() const noexcept -> vk::DescriptorSetLayout
        {
            return _descriptorSetLayout;
        }

        EXAGE_VULKAN_DERIVED

      private:
        std::reference_wrapper<VulkanContext> _context;

        vk::DescriptorPool _descriptorPool;
        vk::DescriptorSetLayout _descriptorSetLayout;
        vk::DescriptorSet _descriptorSet;

        ResourcePool _bufferPool {maxBufferCount};
        ResourcePool _texturePool {maxTextureCount};
    };
}  // namespace exage::Graphics
