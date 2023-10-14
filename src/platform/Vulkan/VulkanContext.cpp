#include "exage/Graphics/Context.h"
#include "exage/Graphics/Error.h"
#include "exage/platform/Vulkan/VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include <exage/platform/GLFW/GLFWWindow.h>
#include <exage/utils/cast.h>

#include "exage/System/Window.h"
#include "exage/platform/Vulkan/VulkanContext.h"

#define GLFW_INCLUDE_VULKAN
#include <iostream>

#include <GLFW/glfw3.h>

#include "exage/platform/Vulkan/VulkanBuffer.h"
#include "exage/platform/Vulkan/VulkanCommandBuffer.h"
#include "exage/platform/Vulkan/VulkanFence.h"
#include "exage/platform/Vulkan/VulkanFrameBuffer.h"
#include "exage/platform/Vulkan/VulkanPipeline.h"
#include "exage/platform/Vulkan/VulkanQueue.h"
#include "exage/platform/Vulkan/VulkanResourceManager.h"
#include "exage/platform/Vulkan/VulkanSampler.h"
#include "exage/platform/Vulkan/VulkanShader.h"
#include "exage/platform/Vulkan/VulkanSwapchain.h"
#include "exage/platform/Vulkan/VulkanTexture.h"
#include "exage/platform/Vulkan/VulkanUtils.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace
{
    template<class InstanceDispatcher, class DeviceDispatcher>
    auto functionsFromDispatcher(InstanceDispatcher const* instance,
                                 DeviceDispatcher const* device) noexcept -> vma::VulkanFunctions
    {
        return vma::VulkanFunctions {
            instance->vkGetInstanceProcAddr,
            instance->vkGetDeviceProcAddr,
            instance->vkGetPhysicalDeviceProperties,
            instance->vkGetPhysicalDeviceMemoryProperties,
            device->vkAllocateMemory,
            device->vkFreeMemory,
            device->vkMapMemory,
            device->vkUnmapMemory,
            device->vkFlushMappedMemoryRanges,
            device->vkInvalidateMappedMemoryRanges,
            device->vkBindBufferMemory,
            device->vkBindImageMemory,
            device->vkGetBufferMemoryRequirements,
            device->vkGetImageMemoryRequirements,
            device->vkCreateBuffer,
            device->vkDestroyBuffer,
            device->vkCreateImage,
            device->vkDestroyImage,
            device->vkCmdCopyBuffer,
            device->vkGetBufferMemoryRequirements2KHR ? device->vkGetBufferMemoryRequirements2KHR
                                                      : device->vkGetBufferMemoryRequirements2,
            device->vkGetImageMemoryRequirements2KHR ? device->vkGetImageMemoryRequirements2KHR
                                                     : device->vkGetImageMemoryRequirements2,
            device->vkBindBufferMemory2KHR ? device->vkBindBufferMemory2KHR
                                           : device->vkBindBufferMemory2,
            device->vkBindImageMemory2KHR ? device->vkBindImageMemory2KHR
                                          : device->vkBindImageMemory2,
            instance->vkGetPhysicalDeviceMemoryProperties2KHR
                ? instance->vkGetPhysicalDeviceMemoryProperties2KHR
                : instance->vkGetPhysicalDeviceMemoryProperties2,
            device->vkGetDeviceBufferMemoryRequirements,
            device->vkGetDeviceImageMemoryRequirements};
    }

    template<class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>
    auto functionsFromDispatcher(
        Dispatch const& dispatch VULKAN_HPP_DEFAULT_DISPATCHER_ASSIGNMENT) noexcept
        -> vma::VulkanFunctions
    {
        return functionsFromDispatcher(&dispatch, &dispatch);
    }
}  // namespace

namespace exage::Graphics
{
    static vk::DynamicLoader dl {};

    auto VulkanContext::create(ContextCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<VulkanContext>, Error>
    {
        std::unique_ptr<VulkanContext> context(new (std::nothrow) VulkanContext());
        tl::expected<void, Error> result = context->init(createInfo);
        if (!result.has_value())
        {
            return tl::make_unexpected(result.error());
        }

        return context;
    }

    auto VulkanContext::init(ContextCreateInfo& createInfo) noexcept -> tl::expected<void, Error>
    {
        auto vkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vkb::InstanceBuilder builder;

        builder.set_app_name("EXAGE");
        builder.require_api_version(1, 2);
        builder.set_engine_name("EXAGE");

#ifdef EXAGE_DEBUG
        builder.request_validation_layers(/*enable_validation=*/true).use_default_debug_messenger();
#endif

        auto inst = builder.build();
        if (!inst)
        {
            return tl::make_unexpected(Errors::UnsupportedAPI {});
        }

        _instance = inst.value();

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance.instance);

        bool const createWindow = createInfo.optionalWindow == nullptr;
        System::Window* window = createInfo.optionalWindow;
        std::unique_ptr<System::Window> windowMemory {};

        if (createWindow)
        {
            constexpr System::WindowInfo info = {
                .name = "Setup Window",
                .extent = {800, 600},
                .fullScreen = false,
                .windowBordered = true,
            };

            tl::expected<std::unique_ptr<System::Window>, System::WindowError> windowRes =
                System::Window::create(info, createInfo.windowAPI);

            debugAssume(windowRes.has_value(), "Failed to create window");
            windowMemory = std::move(windowRes.value());
            window = windowMemory.get();
        }

        auto surface = createSurface(*window);

        vk::PhysicalDeviceFeatures requiredFeatures {};
        requiredFeatures.independentBlend = VK_TRUE;
        requiredFeatures.samplerAnisotropy = VK_TRUE;

        vk::PhysicalDeviceVulkan12Features requiredFeatures12 {};
        requiredFeatures12.drawIndirectCount = VK_TRUE;
        requiredFeatures12.descriptorIndexing = VK_TRUE;
        requiredFeatures12.bufferDeviceAddress = VK_TRUE;
        requiredFeatures12.runtimeDescriptorArray = VK_TRUE;
        requiredFeatures12.descriptorBindingPartiallyBound = VK_TRUE;
        requiredFeatures12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        requiredFeatures12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        requiredFeatures12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        requiredFeatures12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        requiredFeatures12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
        requiredFeatures12.shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE;
        requiredFeatures12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        requiredFeatures12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        requiredFeatures12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        requiredFeatures12.shaderOutputLayer = VK_TRUE;

        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures {};
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        vkb::PhysicalDeviceSelector selector(_instance);
        selector.set_minimum_version(1, 2);
        selector.set_surface(surface);
        selector.add_required_extensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                          VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                          VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
                                          VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                                          VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
                                          VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME});
        selector.set_required_features(requiredFeatures);
        selector.set_required_features_12(requiredFeatures12);
        selector.add_required_extension_features(dynamicRenderingFeatures);

#ifdef EXAGE_DEBUG
        selector.add_desired_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
#endif

        auto phys = selector.select();
        if (!phys)
        {
            return tl::make_unexpected(Errors::UnsupportedAPI {});
        }
        _physicalDevice = phys.value();

        // Check if the device supports the desired extensions
        auto extensionsResult = vk::PhysicalDevice(_physicalDevice.physical_device)
                                    .enumerateDeviceExtensionProperties();
        if (extensionsResult.result != vk::Result::eSuccess)
        {
            return tl::make_unexpected(Errors::UnsupportedAPI {});
        }

        _hardwareSupport.bindlessBuffer = true;
        _hardwareSupport.bindlessTexture = true;
        _hardwareSupport.bufferAddress = true;

#ifdef EXAGE_DEBUG
        std::cout << "Physical Device: " << _physicalDevice.properties.deviceName << std::endl;
#endif

        vkb::DeviceBuilder deviceBuilder(_physicalDevice);

        // vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures {};
        // descriptorIndexingFeatures.sType =
        //     vk::StructureType::ePhysicalDeviceDescriptorIndexingFeatures;
        // descriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        // descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        // descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

        // vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures {};
        // dynamicRenderingFeatures.sType =
        // vk::StructureType::ePhysicalDeviceDynamicRenderingFeatures;
        // dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
        // dynamicRenderingFeatures.setPNext(&descriptorIndexingFeatures);

        // vk::PhysicalDeviceFeatures2 physicalDeviceFeatures {};
        // physicalDeviceFeatures.features = requiredFeatures;

        // deviceBuilder.add_pNext(&physicalDeviceFeatures);
        auto device = deviceBuilder.build();
        debugAssume(device.has_value(), "Failed to create device");
        _device = device.value();

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_device.device);

        vma::AllocatorCreateInfo allocatorInfo {};
        allocatorInfo.physicalDevice = _physicalDevice.physical_device;
        allocatorInfo.device = _device.device;
        allocatorInfo.instance = _instance.instance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;

        vma::VulkanFunctions const functions = functionsFromDispatcher();
        allocatorInfo.pVulkanFunctions = &functions;

        vk::Result const result = vma::createAllocator(&allocatorInfo, &_allocator);
        checkVulkan(result);

        vkb::destroy_surface(_instance, surface);

        vkb::Result<vkb::QueueAndIndex> graphicsQueueAndIndex = _device.get_first_queue_and_index(
            VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
        debugAssume(graphicsQueueAndIndex.has_value(), "Failed to create queue");

        VulkanQueueCreateInfo const queueCreateInfo {
            .maxFramesInFlight = createInfo.maxFramesInFlight,
            .queue = graphicsQueueAndIndex.value().queue,
            .familyIndex = graphicsQueueAndIndex.value().index,
        };

        _queue = VulkanQueue {*this, queueCreateInfo};

        vkb::Result<vkb::QueueAndIndex> transferQueueAndIndex =
            _device.get_preferred_queue_and_index(VK_QUEUE_TRANSFER_BIT);
        debugAssume(transferQueueAndIndex.has_value(), "Failed to create queue");

        VulkanTransferQueueCreateInfo const transferQueueCreateInfo {
            .queue = transferQueueAndIndex.value().queue,
            .familyIndex = transferQueueAndIndex.value().index,
        };

        std::cout << "Transfer Queue Family Index: " << transferQueueAndIndex.value().index
                  << std::endl;

        _transferQueue = VulkanTransferQueue {*this, transferQueueCreateInfo};

        {
            auto depthFormatFirstPick = vk::Format::eD24UnormS8Uint;
            auto depthFormatSecondPick = vk::Format::eD32SfloatS8Uint;

            vk::FormatProperties formatProperties;

            vk::PhysicalDevice physicalDevice = _physicalDevice.physical_device;

            formatProperties = physicalDevice.getFormatProperties(depthFormatFirstPick);
            if (formatProperties.optimalTilingFeatures
                & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
            {
                _hardwareSupport.depthFormat = Format::eDepth24Stencil8;
            }
            else
            {
                formatProperties = physicalDevice.getFormatProperties(depthFormatSecondPick);
                if (formatProperties.optimalTilingFeatures
                    & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                {
                    _hardwareSupport.depthFormat = Format::eDepth32Stencil8;
                }
                else
                {
                    return tl::make_unexpected(Errors::UnsupportedAPI {});
                }
            }
        }

        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = _queue->getFamilyIndex();
        commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        checkVulkan(getDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &_commandPool));

        _resourceManager.emplace(*this);

        return {};
    }

    VulkanContext::~VulkanContext()
    {
        for (auto& pipelineLayout : _pipelineLayoutCache)
        {
            getDevice().destroyPipelineLayout(pipelineLayout.second);
        }

        for (auto& descriptorSetLayout : _descriptorSetLayoutCache)
        {
            getDevice().destroyDescriptorSetLayout(descriptorSetLayout.second);
        }

        getDevice().freeCommandBuffers(_commandPool, _freeCommandBuffers);

        if (_commandPool)
        {
            getDevice().destroyCommandPool(_commandPool);
        }

        _queue = std::nullopt;

        _resourceManager.reset();

        if (_allocator)
        {
            _allocator.destroy();
        }

        if (_device != nullptr)
        {
            vkb::destroy_device(_device);
        }

        if (_instance != nullptr)
        {
            vkb::destroy_instance(_instance);
        }
    }

    void VulkanContext::waitIdle() const noexcept
    {
        checkVulkan(getDevice().waitIdle());
    }

    auto VulkanContext::createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
        -> std::unique_ptr<Swapchain>
    {
        return std::make_unique<VulkanSwapchain>(*this, createInfo);
    }

    auto VulkanContext::createCommandBuffer() noexcept -> std::unique_ptr<CommandBuffer>
    {
        return std::make_unique<VulkanCommandBuffer>(*this);
    }

    auto VulkanContext::createSampler(const SamplerCreateInfo& createInfo) noexcept
        -> std::shared_ptr<Sampler>
    {
        return std::make_shared<VulkanSampler>(*this, createInfo);
    }

    auto VulkanContext::createTexture(const TextureCreateInfo& createInfo) noexcept
        -> std::shared_ptr<Texture>
    {
        return std::make_shared<VulkanTexture>(*this, createInfo);
    }

    auto VulkanContext::createFrameBuffer(glm::uvec2 extent) noexcept
        -> std::shared_ptr<FrameBuffer>
    {
        return std::make_shared<VulkanFrameBuffer>(*this, extent);
    }

    auto VulkanContext::createFrameBuffer(const FrameBufferCreateInfo& createInfo) noexcept
        -> std::shared_ptr<FrameBuffer>
    {
        return std::make_shared<VulkanFrameBuffer>(*this, createInfo);
    }

    auto VulkanContext::createBuffer(const BufferCreateInfo& createInfo) noexcept
        -> std::shared_ptr<Buffer>
    {
        return std::make_shared<VulkanBuffer>(*this, createInfo);
    }

    auto VulkanContext::createShader(const ShaderCreateInfo& createInfo) noexcept
        -> std::shared_ptr<Shader>
    {
        return std::make_shared<VulkanShader>(*this, createInfo);
    }

    auto VulkanContext::createPipeline(const PipelineCreateInfo& createInfo) noexcept
        -> std::shared_ptr<Pipeline>
    {
        return std::make_shared<VulkanPipeline>(*this, createInfo);
    }

    auto VulkanContext::createFence() noexcept -> std::unique_ptr<Fence>
    {
        return std::make_unique<VulkanFence>(*this);
    }

    auto VulkanContext::getHardwareSupport() const noexcept -> HardwareSupport
    {
        return _hardwareSupport;
    }

    auto VulkanContext::getFormatSupport(Format format) const noexcept
        -> std::pair<bool, FormatFeatures>
    {
        vk::Format vkFormat = toVulkanFormat(format);

        vk::FormatProperties formatProperties = getPhysicalDevice().getFormatProperties(vkFormat);

        vk::FormatFeatureFlags features = formatProperties.optimalTilingFeatures;

        constexpr vk::FormatFeatureFlags coreSupport = vk::FormatFeatureFlagBits::eSampledImage
            | vk::FormatFeatureFlagBits::eTransferSrc | vk::FormatFeatureFlagBits::eTransferDst;

        // Ensure that the format is supported with the required features
        if ((features & coreSupport) != coreSupport)
        {
            return {false, {}};
        }

        FormatFeatures formatFeatures {};
        if (features & vk::FormatFeatureFlagBits::eStorageImage)
        {
            formatFeatures |= FormatFeatureFlags::eStorageImage;
        }

        if (features & vk::FormatFeatureFlagBits::eColorAttachment)
        {
            formatFeatures |= FormatFeatureFlags::eColorAttachment;
        }

        return {true, formatFeatures};
    }

    auto VulkanContext::createSurface(System::Window& window) const noexcept -> vk::SurfaceKHR
    {
        VkSurfaceKHR surface = nullptr;
        switch (window.getAPI())
        {
            case System::API::eGLFW:
            {
                const auto* glfWindow = window.as<System::GLFWWindow>();
                debugAssume(glfWindow != nullptr, "Invalid window type");
                glfwCreateWindowSurface(
                    _instance.instance, glfWindow->getGLFWWindow(), nullptr, &surface);
                break;
            }
            default:
                break;
        }

        debugAssume(surface != nullptr, "Surface creation failed");

        return {surface};
    }

    static size_t hashResourceDescriptions(
        const std::vector<ResourceDescription>& resourceDescriptions) noexcept
    {
        size_t hash = 0;
        for (const ResourceDescription& resourceDescription : resourceDescriptions)
        {
            hashCombine(hash, resourceDescription.binding, resourceDescription.type);
        }
        return hash;
    }

    auto VulkanContext::getOrCreateDescriptorSetLayout(
        const std::vector<ResourceDescription>& resourceDescriptions) noexcept
        -> vk::DescriptorSetLayout
    {
        size_t hash = hashResourceDescriptions(resourceDescriptions);
        {
            std::lock_guard<std::mutex> lock(_descriptorSetLayoutCacheMutex);

            const auto it = _descriptorSetLayoutCache.find(hash);
            if (it != _descriptorSetLayoutCache.end())
            {
                return it->second;
            }
        }
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        bindings.reserve(resourceDescriptions.size());
        for (const ResourceDescription& resourceDescription : resourceDescriptions)
        {
            vk::DescriptorSetLayoutBinding binding {};
            binding.binding = resourceDescription.binding;

            switch (resourceDescription.type)
            {
                case ResourceDescription::Type::eSampledImage:
                    binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                    break;
                case ResourceDescription::Type::eStorageImage:
                    binding.descriptorType = vk::DescriptorType::eStorageImage;
                    break;
                case ResourceDescription::Type::eStorageBuffer:
                    binding.descriptorType = vk::DescriptorType::eStorageBuffer;
                    break;
            }

            binding.descriptorCount = 1;
            binding.stageFlags = vk::ShaderStageFlagBits::eAll;
            binding.pImmutableSamplers = nullptr;

            bindings.push_back(binding);
        }

        vk::DescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;

        vk::DescriptorSetLayout layout;
        checkVulkan(getDevice().createDescriptorSetLayout(&layoutInfo, nullptr, &layout));
        {
            std::lock_guard<std::mutex> lock(_descriptorSetLayoutCacheMutex);
            _descriptorSetLayoutCache[hash] = layout;
        }
        return layout;
    }

    static auto hashPipelineLayoutInfo(const VulkanContext::PipelineLayoutInfo& info) noexcept
        -> size_t
    {
        size_t seed = 0;
        hashCombine(seed,
                    info.pushConstantSize,
                    hashResourceDescriptions(info.resourceDescriptions),
                    info.bindless);
        return seed;
    }

    auto VulkanContext::getOrCreatePipelineLayout(const PipelineLayoutInfo& info) noexcept
        -> vk::PipelineLayout
    {
        size_t hash = hashPipelineLayoutInfo(info);
        {
            std::lock_guard<std::mutex> lock(_pipelineLayoutCacheMutex);

            const auto it = _pipelineLayoutCache.find(hash);
            if (it != _pipelineLayoutCache.end())
            {
                return it->second;
            }
        }

        vk::DescriptorSetLayout layout;
        if (info.bindless)
        {
            layout = _resourceManager->getDescriptorSetLayout();
        }
        else
        {
            layout = getOrCreateDescriptorSetLayout(info.resourceDescriptions);
        }

        vk::PushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eAll;
        pushConstantRange.offset = 0;
        pushConstantRange.size = static_cast<uint32_t>(info.pushConstantSize);

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.setSetLayouts(layout);
        pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);

        vk::PipelineLayout pipelineLayout;
        checkVulkan(
            getDevice().createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout));
        {
            std::lock_guard<std::mutex> lock(_pipelineLayoutCacheMutex);
            _pipelineLayoutCache[hash] = pipelineLayout;
        }
        return pipelineLayout;
    }

    auto VulkanContext::getDevice() const noexcept -> vk::Device
    {
        return _device.device;
    }

    auto VulkanContext::getAllocator() const noexcept -> vma::Allocator
    {
        return _allocator;
    }

    auto VulkanContext::getVulkanBootstrapDevice() const noexcept -> vkb::Device
    {
        return _device;
    }

    auto VulkanContext::getInstance() const noexcept -> vk::Instance
    {
        return _instance.instance;
    }

    auto VulkanContext::getPhysicalDevice() const noexcept -> vk::PhysicalDevice
    {
        return _physicalDevice.physical_device;
    }

    auto VulkanContext::createVulkanCommandBuffer() noexcept -> vk::CommandBuffer
    {
        std::lock_guard<std::mutex> lock(_commandPoolMutex);

        if (!_freeCommandBuffers.empty())
        {
            vk::CommandBuffer commandBuffer = _freeCommandBuffers.back();
            _freeCommandBuffers.pop_back();
            return commandBuffer;
        }

        vk::CommandBufferAllocateInfo allocInfo {};
        allocInfo.commandPool = _commandPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        checkVulkan(getDevice().allocateCommandBuffers(&allocInfo, &commandBuffer));
        return commandBuffer;
    }

    void VulkanContext::destroyCommandBuffer(vk::CommandBuffer commandBuffer) noexcept
    {
        std::lock_guard<std::mutex> lock(_commandPoolMutex);

        commandBuffer.reset();
        _freeCommandBuffers.push_back(commandBuffer);
    }

}  // namespace exage::Graphics
