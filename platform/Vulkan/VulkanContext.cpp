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

#include "VulkanSwapchain.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace exage::Graphics
{
    static vk::DynamicLoader dl{};


    tl::expected<std::unique_ptr<Context>, Error> VulkanContext::create(
        ContextCreateInfo& createInfo) noexcept
    {
        std::unique_ptr<VulkanContext> context(new VulkanContext());
        std::optional<Error> result = context->init(createInfo);
        if (result)
        {
            return tl::make_unexpected(result.value());
        }

        return context;
    }

    auto VulkanContext::init(ContextCreateInfo& createInfo) noexcept -> std::optional<Error>
    {
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vkb::InstanceBuilder builder;

        builder.set_app_name("EXAGE");

#ifdef EXAGE_DEBUG
        builder.request_validation_layers(true).use_default_debug_messenger();
#endif

        auto inst = builder.build();
        if (!inst)
        {
            return ErrorCode::eInstanceCreationFailed;
        }

        _instance = inst.value();

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance.instance);

        constexpr WindowInfo info = {
            .extent = {800, 600},
            .name = "Setup Window",
            .fullScreenMode = FullScreenMode::eWindowed,
        };

        tl::expected<std::unique_ptr<Window>, WindowError> window = Window::create(
            info,
            createInfo.windowAPI);
        if (!window)
        {
            return ErrorCode::eInvalidWindow;
        }

        auto surfaceResult = createSurface(*window.value());
        if (!surfaceResult)
        {
            return surfaceResult.error();
        }
        auto surface = surfaceResult.value();

        vkb::PhysicalDeviceSelector selector(_instance);
        selector.set_minimum_version(1, 3);
        selector.set_surface(surface);
        selector.add_required_extensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                          VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                          VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
                                          VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                                          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                          VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME});
        selector.add_desired_extensions({VK_EXT_MESH_SHADER_EXTENSION_NAME});

        auto phys = selector.select();
        if (!phys)
        {
            return ErrorCode::ePhysicalDeviceSelectionFailed;
        }
        _physicalDevice = phys.value();

        vkb::DeviceBuilder deviceBuilder(_physicalDevice);

        vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
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

        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
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
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

        vk::Result result = vma::createAllocator(&allocatorInfo, &_allocator);
        if (result != vk::Result::eSuccess)
        {
            return ErrorCode::eAllocatorCreationFailed;
        }

        auto graphicsQueue = _device.get_queue(vkb::QueueType::graphics).value();

        vkb::destroy_surface(_instance, surface);

        return std::nullopt;
    }


    VulkanContext::~VulkanContext()
    {
        vkb::destroy_device(_device);
        vkb::destroy_instance(_instance);
    }

    void VulkanContext::waitIdle() const noexcept
    {
        getDevice().waitIdle();
    }

    auto VulkanContext::createSwapchain(const SwapchainCreateInfo& createInfo) noexcept
    -> tl::expected<std::unique_ptr<Swapchain>, Error>
    {
        return VulkanSwapchain::create(*this, createInfo);
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

    auto VulkanContext::getInstance() const noexcept -> vk::Instance
    {
        return _instance.instance;
    }

    auto VulkanContext::getPhysicalDevice() const noexcept -> vk::PhysicalDevice
    {
        return _physicalDevice.physical_device;
    }
} // namespace exage::Graphics
