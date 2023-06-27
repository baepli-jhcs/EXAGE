#include "exage/platform/Vulkan/VulkanResourceManager.h"

#include "exage/platform/Vulkan/VulkanBuffer.h"
#include "exage/platform/Vulkan/VulkanSampler.h"
#include "exage/platform/Vulkan/VulkanTexture.h"

namespace exage::Graphics
{
    VulkanResourceManager::VulkanResourceManager(VulkanContext& context) noexcept
        : _context(context)
    {
        _support = _context.get().getHardwareSupport().bindlessBuffer
            && _context.get().getHardwareSupport().bindlessTexture;

        if (!_support)
        {
            return;
        }

        vk::DescriptorPoolSize const bufferDescriptorPoolSize {vk::DescriptorType::eStorageBuffer,
                                                               maxBufferCount};

        vk::DescriptorPoolSize const samplerDescriptorPoolSize {vk::DescriptorType::eSampler,
                                                                maxSamplerCount};

        vk::DescriptorPoolSize const sampledImageDescriptorPoolSize {
            vk::DescriptorType::eSampledImage, maxTextureCount};

        vk::DescriptorPoolSize const storageImageDescriptorPoolSize {
            vk::DescriptorType::eStorageImage, maxTextureCount};

        std::array descriptorPoolSizes {sampledImageDescriptorPoolSize,
                                        bufferDescriptorPoolSize,
                                        storageImageDescriptorPoolSize};

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {};
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet
            | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.setPoolSizes(descriptorPoolSizes);

        checkVulkan(_context.get().getDevice().createDescriptorPool(
            &descriptorPoolCreateInfo, nullptr, &_descriptorPool));

        vk::DescriptorSetLayoutBinding storageBufferDescriptorSetLayoutBinding {};
        storageBufferDescriptorSetLayoutBinding.binding = storageBufferBinding;
        storageBufferDescriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
        storageBufferDescriptorSetLayoutBinding.descriptorCount = maxBufferCount;
        storageBufferDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        vk::DescriptorSetLayoutBinding samplerDescriptorSetLayoutBinding {};
        samplerDescriptorSetLayoutBinding.binding = samplerBinding;
        samplerDescriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eSampler;
        samplerDescriptorSetLayoutBinding.descriptorCount = maxSamplerCount;
        samplerDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        vk::DescriptorSetLayoutBinding sampledImageSetLayoutBinding {};
        sampledImageSetLayoutBinding.binding = sampledTextureBinding;
        sampledImageSetLayoutBinding.descriptorType = vk::DescriptorType::eSampledImage;
        sampledImageSetLayoutBinding.descriptorCount = maxTextureCount;
        sampledImageSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        // vk::DescriptorSetLayoutBinding uniformBufferDescriptorSetLayoutBinding {};
        // uniformBufferDescriptorSetLayoutBinding.binding = uniformBufferBinding;
        // uniformBufferDescriptorSetLayoutBinding.descriptorType =
        // vk::DescriptorType::eUniformBuffer;
        // uniformBufferDescriptorSetLayoutBinding.descriptorCount = maxBufferCount;
        // uniformBufferDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        vk::DescriptorSetLayoutBinding storageImageDescriptorSetLayoutBinding {};
        storageImageDescriptorSetLayoutBinding.binding = storageTextureBinding;
        storageImageDescriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
        storageImageDescriptorSetLayoutBinding.descriptorCount = maxTextureCount;
        storageImageDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        std::array<vk::DescriptorSetLayoutBinding, 4> descriptorSetLayoutBindings {
            storageBufferDescriptorSetLayoutBinding,
            // uniformBufferDescriptorSetLayoutBinding,
            samplerDescriptorSetLayoutBinding,
            sampledImageSetLayoutBinding,
            storageImageDescriptorSetLayoutBinding};

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
        descriptorSetLayoutCreateInfo.setBindings(descriptorSetLayoutBindings);
        descriptorSetLayoutCreateInfo.flags =
            vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

        std::array<vk::DescriptorBindingFlags, 4> descriptorBindingFlags {};
        descriptorBindingFlags[0] = vk::DescriptorBindingFlagBits::eUpdateAfterBind
            | vk::DescriptorBindingFlagBits::ePartiallyBound
            | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
        descriptorBindingFlags[1] = descriptorBindingFlags[0];
        descriptorBindingFlags[2] = descriptorBindingFlags[0];
        descriptorBindingFlags[3] = descriptorBindingFlags[0];

        vk::DescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo {};
        descriptorSetLayoutBindingFlagsCreateInfo.setBindingFlags(descriptorBindingFlags);

        descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;

        checkVulkan(_context.get().getDevice().createDescriptorSetLayout(
            &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout));

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo {};
        descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.setSetLayouts(_descriptorSetLayout);

        checkVulkan(_context.get().getDevice().allocateDescriptorSets(&descriptorSetAllocateInfo,
                                                                      &_descriptorSet));
    }

    VulkanResourceManager::~VulkanResourceManager()
    {
        if (!_support)
        {
            return;
        }

        _context.get().getDevice().freeDescriptorSets(_descriptorPool, _descriptorSet);

        _context.get().getDevice().destroyDescriptorSetLayout(_descriptorSetLayout);

        _context.get().getDevice().destroyDescriptorPool(_descriptorPool);
    }

    auto VulkanResourceManager::bindBuffer(VulkanBuffer& buffer) noexcept -> BufferID
    {
        if (!_support)
        {
            return BufferID {};
        }

        vk::DescriptorBufferInfo descriptorBufferInfo {};
        descriptorBufferInfo.buffer = buffer.getBuffer();
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = buffer.getSize();

        uint32_t const index = _bufferPool.allocate();

        vk::WriteDescriptorSet writeDescriptorSet {};
        writeDescriptorSet.dstSet = _descriptorSet;
        writeDescriptorSet.dstBinding = storageBufferBinding;
        writeDescriptorSet.dstArrayElement = index;
        writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageBuffer;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

        _context.get().getDevice().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);

        return BufferID {index};
    }

    auto VulkanResourceManager::bindTextureWith(vk::ImageView imageView,
                                                vk::ImageLayout layout,
                                                bool sampled,
                                                bool storage) noexcept -> TextureID
    {
        if (!_support)
        {
            return TextureID {};
        }

        uint32_t const index = _texturePool.allocate();

        vk::WriteDescriptorSet sampledImageWriteDescriptorSet {};
        vk::DescriptorImageInfo sampledImageInfo = {
            nullptr, imageView, vk::ImageLayout::eUndefined};
        if (sampled)
        {
            sampledImageInfo.imageLayout = layout;
            sampledImageWriteDescriptorSet.dstSet = _descriptorSet;
            sampledImageWriteDescriptorSet.dstBinding = sampledTextureBinding;
            sampledImageWriteDescriptorSet.dstArrayElement = index;
            sampledImageWriteDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
            sampledImageWriteDescriptorSet.descriptorCount = 1;
            sampledImageWriteDescriptorSet.pImageInfo = &sampledImageInfo;
        }

        vk::WriteDescriptorSet storageImageWriteDescriptorSet {};
        vk::DescriptorImageInfo storageImageDescriptorImageInfo = sampledImageInfo;
        if (storage)
        {
            storageImageDescriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
            storageImageWriteDescriptorSet.dstSet = _descriptorSet;
            storageImageWriteDescriptorSet.dstBinding = storageTextureBinding;
            storageImageWriteDescriptorSet.dstArrayElement = index;
            storageImageWriteDescriptorSet.descriptorType = vk::DescriptorType::eStorageImage;
            storageImageWriteDescriptorSet.descriptorCount = 1;
            storageImageWriteDescriptorSet.pImageInfo = &storageImageDescriptorImageInfo;
        }

        std::array<vk::WriteDescriptorSet, 2> writeDescriptorSets {};
        uint32_t writeDescriptorSetCount = 0;
        if (sampled)
        {
            writeDescriptorSets[writeDescriptorSetCount] = sampledImageWriteDescriptorSet;
            writeDescriptorSetCount++;
        }
        if (storage)
        {
            writeDescriptorSets[writeDescriptorSetCount] = storageImageWriteDescriptorSet;
            writeDescriptorSetCount++;
        }

        _context.get().getDevice().updateDescriptorSets(
            writeDescriptorSetCount, &sampledImageWriteDescriptorSet, 0, nullptr);

        return TextureID {index};
    }

    auto VulkanResourceManager::bindTexture(VulkanTexture& texture) noexcept -> TextureID
    {
        bool sampledImage = texture.getUsage().any(Texture::UsageFlags::eSampled);
        bool storageImage = texture.getUsage().any(Texture::UsageFlags::eStorage);

        return bindTextureWith(texture.getImageView(Texture::Aspect::eColor),
                               vk::ImageLayout::eShaderReadOnlyOptimal,
                               sampledImage,
                               storageImage);
    }

    auto VulkanResourceManager::bindDepthStencilTexture(VulkanTexture& texture) noexcept
        -> std::pair<TextureID, TextureID>
    {
        if (!_support)
        {
            return {TextureID {}, TextureID {}};
        }

        bool sampledImage = texture.getUsage().any(Texture::UsageFlags::eSampled);
        bool storageImage = texture.getUsage().any(Texture::UsageFlags::eStorage);

        TextureID depthTextureID = bindTextureWith(texture.getImageView(Texture::Aspect::eDepth),
                                                   vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                                                   sampledImage,
                                                   storageImage);
        TextureID stencilTextureID =
            bindTextureWith(texture.getImageView(Texture::Aspect::eStencil),
                            vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                            sampledImage,
                            storageImage);

        return {depthTextureID, stencilTextureID};
    }

    auto VulkanResourceManager::bindSampler(VulkanSampler& sampler) noexcept -> SamplerID
    {
        if (!_support)
        {
            return SamplerID {};
        }

        vk::DescriptorImageInfo descriptorImageInfo {};
        descriptorImageInfo.sampler = sampler.getSampler();

        uint32_t const index = _samplerPool.allocate();

        vk::WriteDescriptorSet writeDescriptorSet {};
        writeDescriptorSet.dstSet = _descriptorSet;
        writeDescriptorSet.dstBinding = samplerBinding;
        writeDescriptorSet.dstArrayElement = index;
        writeDescriptorSet.descriptorType = vk::DescriptorType::eSampler;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &descriptorImageInfo;

        _context.get().getDevice().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);

        return SamplerID {index};
    }

    void VulkanResourceManager::unbindBuffer(BufferID buffer) noexcept
    {
        if (!_support)
        {
            return;
        }

        _bufferPool.free(buffer.id);
    }

    void VulkanResourceManager::unbindTexture(TextureID texture) noexcept
    {
        if (!_support)
        {
            return;
        }

        _texturePool.free(texture.id);
    }

    void VulkanResourceManager::unbindSampler(SamplerID sampler) noexcept
    {
        if (!_support)
        {
            return;
        }

        _samplerPool.free(sampler.id);
    }

}  // namespace exage::Graphics
