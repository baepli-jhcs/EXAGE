#pragma once

#include <memory>
#include <mutex>

#include "exage/Graphics/BindlessResources.h"
#include "exage/platform/Vulkan/VKinclude.h"

namespace exage::Graphics
{
    namespace detail
    {
        class VulkanResourcePool
        {
          public:
            explicit VulkanResourcePool(uint32_t maxResourceCount) noexcept
            {
                freeSlots.resize(maxResourceCount);
                for (uint32_t i = 0; i < maxResourceCount; ++i)
                {
                    freeSlots[i] = i;
                }
            }

            EXAGE_DELETE_COPY(VulkanResourcePool);
            EXAGE_DELETE_MOVE(VulkanResourcePool);

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

    class VulkanContext;
    class VulkanBuffer;
    class VulkanTexture;
    class VulkanSampler;

    class VulkanResourceManager
    {
      public:
        using ResourcePool = detail::VulkanResourcePool;

        constexpr static uint32_t maxBufferCount = 65536;
        constexpr static uint32_t maxTextureCount = 65536;
        constexpr static uint32_t maxSamplerCount = 65536;

        constexpr static uint32_t storageBufferBinding = 0;
        constexpr static uint32_t samplerBinding = 1;
        constexpr static uint32_t sampledTextureBinding = 2;
        constexpr static uint32_t storageTextureBinding = 3;

        explicit VulkanResourceManager(VulkanContext& context) noexcept;
        ~VulkanResourceManager();

        EXAGE_DELETE_COPY(VulkanResourceManager);
        EXAGE_DELETE_MOVE(VulkanResourceManager);

        [[nodiscard]] auto bindBuffer(VulkanBuffer& buffer) noexcept -> BufferID;
        [[nodiscard]] auto bindTexture(VulkanTexture& texture) noexcept -> TextureID;
        [[nodiscard]] auto bindDepthStencilTexture(VulkanTexture& texture) noexcept
            -> std::pair<TextureID, TextureID>;  // depth, stencil
        [[nodiscard]] auto bindSampler(VulkanSampler& sampler) noexcept -> SamplerID;

        void unbindBuffer(BufferID buffer) noexcept;
        void unbindTexture(TextureID texture) noexcept;
        void unbindSampler(SamplerID sampler) noexcept;

        [[nodiscard]] auto getDescriptorSet() const noexcept -> vk::DescriptorSet
        {
            return _descriptorSet;
        }
        [[nodiscard]] auto getDescriptorSetLayout() const noexcept -> vk::DescriptorSetLayout
        {
            return _descriptorSetLayout;
        }

      private:
        [[nodiscard]] auto bindTextureWith(vk::ImageView imageView,
                                           vk::ImageLayout layout,
                                           bool sampled,
                                           bool storage) noexcept -> TextureID;

        std::reference_wrapper<VulkanContext> _context;

        bool _support;

        vk::DescriptorPool _descriptorPool;
        vk::DescriptorSetLayout _descriptorSetLayout;
        vk::DescriptorSet _descriptorSet;

        ResourcePool _bufferPool {maxBufferCount};
        ResourcePool _texturePool {maxTextureCount};
        ResourcePool _samplerPool {maxSamplerCount};
    };
}  // namespace exage::Graphics
