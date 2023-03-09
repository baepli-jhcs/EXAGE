#define VMA_IMPLEMENTATION
#include "VulkanContext.h"

#include <GLFW/GLFWindow.h>
#include <utils/cast.h>

#include "Core/Window.h"
#include "VkBootstrap.h"
#include "Vulkan/VulkanContext.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanTexture.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanSwapchain.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace
{
    template<class InstanceDispatcher, class DeviceDispatcher>
    vma::VulkanFunctions functionsFromDispatcher(InstanceDispatcher const* instance,
                                                 DeviceDispatcher const* device) noexcept
    {
        return vma::VulkanFunctions{
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
            device->vkGetBufferMemoryRequirements2KHR
            ? device->vkGetBufferMemoryRequirements2KHR
            : device->vkGetBufferMemoryRequirements2,
            device->vkGetImageMemoryRequirements2KHR
            ? device->vkGetImageMemoryRequirements2KHR
            : device->vkGetImageMemoryRequirements2,
            device->vkBindBufferMemory2KHR
            ? device->vkBindBufferMemory2KHR
            : device->vkBindBufferMemory2,
            device->vkBindImageMemory2KHR
            ? device->vkBindImageMemory2KHR
            : device->vkBindImageMemory2,
            instance->vkGetPhysicalDeviceMemoryProperties2KHR
            ? instance->vkGetPhysicalDeviceMemoryProperties2KHR
            : instance->vkGetPhysicalDeviceMemoryProperties2,
            device->vkGetDeviceBufferMemoryRequirements,
            device->vkGetDeviceImageMemoryRequirements};
    }

    template<class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>
    vma::VulkanFunctions functionsFromDispatcher(
        Dispatch const& dispatch VULKAN_HPP_DEFAULT_DISPATCHER_ASSIGNMENT) noexcept
    {
        return functionsFromDispatcher(&dispatch, &dispatch);
    }
} // namespace

namespace exage::Graphics
{
    static vk::DynamicLoader dl{};


    tl::expected<VulkanContext, Error> VulkanContext::create(
        ContextCreateInfo& createInfo) noexcept
    {
        VulkanContext context{};
        std::optional<Error> result = context.init(createInfo);
        if (result.has_value())
        {
            return tl::make_unexpected(result.value());
        }

        return context;
    }

    auto VulkanContext::init(ContextCreateInfo& createInfo) noexcept -> std::optional<Error>
    {
        auto vkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vkb::InstanceBuilder builder;

        builder.set_app_name("EXAGE");

#ifdef EXAGE_DEBUG
        builder.request_validation_layers(/*enable_validation=*/true).use_default_debug_messenger();
#endif

        auto inst = builder.build();
        if (!inst)
        {
            return ErrorCode::eInstanceCreationFailed;
        }

        _instance = inst.value();

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance.instance);

        bool createWindow = createInfo.optionalWindow == nullptr;
        Window* window = createInfo.optionalWindow;
        std::unique_ptr<Window> windowMemory{};

        if (createWindow)
        {
            constexpr WindowInfo info = {
                .extent = {800, 600},
                .name = "Setup Window",
                .fullScreenMode = FullScreenMode::eWindowed,
            };

            tl::expected<std::unique_ptr<Window>, WindowError> windowRes =
                Window::create(info, createInfo.windowAPI);

            if (!windowRes)
            {
                return ErrorCode::eInvalidWindow;
            }

            windowMemory = std::move(windowRes.value());
            window = windowMemory.get();
        }

        auto surfaceResult = createSurface(*window);
        if (!surfaceResult)
        {
            return surfaceResult.error();
        }
        auto surface = surfaceResult.value();

        vkb::PhysicalDeviceSelector selector(_instance);
        selector.set_minimum_version(1, 2);
        selector.set_surface(surface);
        selector.add_required_extensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                          VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                          VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
                                          VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                                          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                          VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME});
        //selector.add_desired_extensions({VK_EXT_MESH_SHADER_EXTENSION_NAME});

        auto phys = selector.select();
        if (!phys)
        {
            return ErrorCode::ePhysicalDeviceSelectionFailed;
        }
        _physicalDevice = phys.value();

        vkb::DeviceBuilder deviceBuilder(_physicalDevice);

        vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{};
        descriptorIndexingFeatures.sType =
            vk::StructureType::ePhysicalDeviceDescriptorIndexingFeatures;
        descriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

        vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = vk::StructureType::ePhysicalDeviceDynamicRenderingFeatures;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
        descriptorIndexingFeatures.setPNext(&dynamicRenderingFeatures);

        deviceBuilder.add_pNext(&descriptorIndexingFeatures);
        auto device = deviceBuilder.build();
        if (!device)
        {
            return ErrorCode::eDeviceCreationFailed;
        }
        _device = device.value();

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_device.device);

        vma::AllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = _physicalDevice.physical_device;
        allocatorInfo.device = _device.device;
        allocatorInfo.instance = _instance.instance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;

        vma::VulkanFunctions functions = functionsFromDispatcher();
        allocatorInfo.pVulkanFunctions = &functions;

        vk::Result result = vma::createAllocator(&allocatorInfo, &_allocator);
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eAllocatorCreationFailed;
        }

        vkb::destroy_surface(_instance, surface);

        return std::nullopt;
    }


    VulkanContext::~VulkanContext()
    {
        if (_allocator)
        {
            _allocator.destroy();
        }

        if (_device)
        {
            vkb::destroy_device(_device);
        }

        if (_instance)
        {
            vkb::destroy_instance(_instance);
        }
    }

    VulkanContext::VulkanContext(VulkanContext&& old) noexcept
    {
        *this = std::move(old);
    }

    auto VulkanContext::operator=(VulkanContext&& old) noexcept -> VulkanContext&
    {
        _instance = old._instance;
        _physicalDevice = old._physicalDevice;
        _device = old._device;
        _allocator = old._allocator;

        old._instance = {};
        old._physicalDevice = {};
        old._device = {};
        old._allocator = nullptr;

        return *this;
    }

    void VulkanContext::waitIdle() const noexcept
    {
        getDevice().waitIdle();
    }

    auto VulkanContext::createQueue(
        const QueueCreateInfo& createInfo) noexcept -> tl::expected<std::unique_ptr<Queue>, Error>
    {
        tl::expected value = VulkanQueue::create(*this, createInfo);
        if (!value.has_value())
        {
            return tl::make_unexpected(value.error());
        }

        return std::make_unique<VulkanQueue>(std::move(value.value()));
    }

    auto VulkanContext::createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
    -> tl::expected<std::unique_ptr<Swapchain>, Error>
    {
        tl::expected value = VulkanSwapchain::create(*this, createInfo);
        if (!value.has_value())
        {
            return tl::make_unexpected(value.error());
        }
        return std::make_unique<VulkanSwapchain>(std::move(value.value()));
    }

    auto VulkanContext::createCommandBuffer() noexcept
    -> tl::expected<std::unique_ptr<CommandBuffer>, Error>
    {
        tl::expected value = VulkanCommandBuffer::create(*this);
        if (!value.has_value())
        {
            return tl::make_unexpected(value.error());
        }
        return std::make_unique<VulkanCommandBuffer>(std::move(value.value()));
    }

    auto VulkanContext::createTexture(const TextureCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<Texture>, Error>
    {
        tl::expected value = VulkanTexture::create(*this, createInfo);
        if (!value.has_value())
        {
			return tl::make_unexpected(value.error());
		}
		return std::make_unique<VulkanTexture>(std::move(value.value()));
    }

    auto VulkanContext::createSurface(Window& window) const noexcept
    -> tl::expected<vk::SurfaceKHR, Error>
    {
        VkSurfaceKHR surface = nullptr;
        switch (window.getAPI())
        {
            case WindowAPI::eGLFW:
            {
                const auto* glfWindow = dynamicCast<GLFWindow*>(&window);
                if (!glfWindow)
                {
                    return tl::make_unexpected(ErrorCode::eInvalidWindow);
                }
                glfwCreateWindowSurface(
                    _instance.instance,
                    glfWindow->getGLFWWindow(),
                    nullptr,
                    &surface);
                break;
            }
            default:
                return tl::make_unexpected(ErrorCode::eInvalidWindow);
        }

        if (surface == nullptr)
        {
            return tl::make_unexpected(ErrorCode::eSurfaceCreationFailed);
        }

        return vk::SurfaceKHR(surface);
    }

    auto VulkanContext::getDevice() const noexcept -> vk::Device
    {
        return _device.device;
    }

    auto VulkanContext::getAllocator() const noexcept -> vma::Allocator
    {
        return _allocator;
    }

    auto VulkanContext::getQueueIndex() const noexcept -> uint32_t
    {
        return _device.get_queue_index(vkb::QueueType::graphics).value();
    }

    auto VulkanContext::getVulkanQueue() const noexcept -> vk::Queue
    {
        return _device.get_queue(vkb::QueueType::graphics).value();
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
} // namespace exage::Graphics
