#include "exage/platform/Vulkan/VulkanResourceManager.h"

#include "exage/platform/Vulkan/VulkanBuffer.h"
#include "exage/platform/Vulkan/VulkanTexture.h"

namespace exage::Graphics
{
    VulkanResourceManager::VulkanResourceManager(VulkanContext& context) noexcept
        : _context(context)
    {
        vk::DescriptorPoolSize const bufferDescriptorPoolSize {vk::DescriptorType::eStorageBuffer,
                                                               maxBufferCount};

        vk::DescriptorPoolSize const combinedImageSamplerDescriptorPoolSize {
            vk::DescriptorType::eCombinedImageSampler, maxTextureCount};

        std::array descriptorPoolSizes {bufferDescriptorPoolSize,
                                        combinedImageSamplerDescriptorPoolSize};

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {};
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet
            | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.setPoolSizes(descriptorPoolSizes);

        checkVulkan(_context.get().getDevice().createDescriptorPool(
            &descriptorPoolCreateInfo, nullptr, &_descriptorPool));

        vk::DescriptorSetLayoutBinding combinedImageSamplerSetLayoutBinding {};
        combinedImageSamplerSetLayoutBinding.binding = sampledTextureBinding;
        combinedImageSamplerSetLayoutBinding.descriptorType =
            vk::DescriptorType::eCombinedImageSampler;
        combinedImageSamplerSetLayoutBinding.descriptorCount = maxTextureCount;
        combinedImageSamplerSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        vk::DescriptorSetLayoutBinding storageBufferDescriptorSetLayoutBinding {};
        storageBufferDescriptorSetLayoutBinding.binding = storageBufferBinding;
        storageBufferDescriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
        storageBufferDescriptorSetLayoutBinding.descriptorCount = maxBufferCount;
        storageBufferDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        //vk::DescriptorSetLayoutBinding uniformBufferDescriptorSetLayoutBinding {};
        //uniformBufferDescriptorSetLayoutBinding.binding = uniformBufferBinding;
        //uniformBufferDescriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        //uniformBufferDescriptorSetLayoutBinding.descriptorCount = maxBufferCount;
        //uniformBufferDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        vk::DescriptorSetLayoutBinding storageImageDescriptorSetLayoutBinding {};
        storageImageDescriptorSetLayoutBinding.binding = storageTextureBinding;
        storageImageDescriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
        storageImageDescriptorSetLayoutBinding.descriptorCount = maxTextureCount;
        storageImageDescriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAll;

        std::array descriptorSetLayoutBindings {combinedImageSamplerSetLayoutBinding,
                                                storageBufferDescriptorSetLayoutBinding,
                                                //uniformBufferDescriptorSetLayoutBinding,
                                                storageImageDescriptorSetLayoutBinding};

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
        descriptorSetLayoutCreateInfo.setBindings(descriptorSetLayoutBindings);
        descriptorSetLayoutCreateInfo.flags =
            vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

        std::array<vk::DescriptorBindingFlags, 3> descriptorBindingFlags {};
        descriptorBindingFlags[0] = vk::DescriptorBindingFlagBits::eUpdateAfterBind
            | vk::DescriptorBindingFlagBits::ePartiallyBound
            | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
        descriptorBindingFlags[1] = descriptorBindingFlags[0];
        descriptorBindingFlags[2] = descriptorBindingFlags[0];

        vk::DescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo {};
        descriptorSetLayoutBindingFlagsCreateInfo.setBindingFlags(descriptorBindingFlags);

        descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;

        checkVulkan(_context.get().getDevice().createDescriptorSetLayout(
            &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout));

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo {};
        descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
        descriptorSetAllocateInfo.setSetLayouts(_descriptorSetLayout);

        checkVulkan(_context.get().getDevice().allocateDescriptorSets(&descriptorSetAllocateInfo,
                                                                      &_descriptorSet));
    }

    VulkanResourceManager::~VulkanResourceManager()
    {
        _context.get().getDevice().destroyDescriptorPool(_descriptorPool);
        _context.get().getDevice().destroyDescriptorSetLayout(_descriptorSetLayout);
        _context.get().getDevice().freeDescriptorSets(_descriptorPool, _descriptorSet);
    }

    auto VulkanResourceManager::bindBuffer(Buffer& buffer) noexcept -> BufferID
    {
        const auto* vulkanBuffer = buffer.as<VulkanBuffer>();

        vk::DescriptorBufferInfo descriptorBufferInfo {};
        descriptorBufferInfo.buffer = vulkanBuffer->getBuffer();
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = vulkanBuffer->getSize();

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

    auto VulkanResourceManager::bindTexture(Texture& texture) noexcept -> TextureID
    {
        const auto* vulkanTexture = texture.as<VulkanTexture>();

        vk::DescriptorImageInfo const imageInfo = vulkanTexture->getDescriptorImageInfo();

        uint32_t const index = _texturePool.allocate();

        vk::WriteDescriptorSet writeDescriptorSet {};
        writeDescriptorSet.dstSet = _descriptorSet;
        writeDescriptorSet.dstBinding = sampledTextureBinding;
        writeDescriptorSet.dstArrayElement = index;
        writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &imageInfo;

        vk::WriteDescriptorSet storageImageWriteDescriptorSet {};
        vk::DescriptorImageInfo storageImageDescriptorImageInfo = imageInfo;
        bool storageImage = vulkanTexture->getUsage().any(Texture::UsageFlags::eStorage); 
        if (storageImage)
        {
            storageImageDescriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;
			storageImageWriteDescriptorSet.dstSet = _descriptorSet;
			storageImageWriteDescriptorSet.dstBinding = storageTextureBinding;
			storageImageWriteDescriptorSet.dstArrayElement = index;
			storageImageWriteDescriptorSet.descriptorType = vk::DescriptorType::eStorageImage;
			storageImageWriteDescriptorSet.descriptorCount = 1;
			storageImageWriteDescriptorSet.pImageInfo = &storageImageDescriptorImageInfo;
        }

        std::array writeDescriptorSets {writeDescriptorSet, storageImageWriteDescriptorSet};
        uint32_t writeDescriptorSetCount = storageImage ? 2 : 1;

        _context.get().getDevice().updateDescriptorSets(writeDescriptorSetCount, &writeDescriptorSet, 0, nullptr);

        return TextureID {index};
    }

    void VulkanResourceManager::unbindBuffer(BufferID buffer) noexcept
    {
        _bufferPool.free(buffer.id);
    }

    void VulkanResourceManager::unbindTexture(TextureID texture) noexcept
    {
        _texturePool.free(texture.id);
    }

}  // namespace exage::Graphics
