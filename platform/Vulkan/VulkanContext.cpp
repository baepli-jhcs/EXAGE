#include "VulkanContext.h"

#include <GLFW/GLFWindow.h>
#include <utils/cast.h>
#include <volk.h>

#include "Core/Window.h"
#include "VkBootstrap.h"
#include "Vulkan/VulkanContext.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace exage::Graphics
{
    VulkanContext::VulkanContext(WindowAPI windowAPI)
    {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to initialize volk");
        }

        vkb::InstanceBuilder builder;

        builder.set_app_name("EXAGE");

#ifdef EXAGE_DEBUG
        builder.request_validation_layers(true).use_default_debug_messenger();
#endif

        auto inst = builder.build();
        if (!inst)
        {
            throw std::runtime_error("Failed to create Vulkan instance");
        }

        _instance = inst.value();

        volkLoadInstance(_instance.instance);

        VkSurfaceKHR surface = VK_NULL_HANDLE;

        constexpr WindowInfo info = {
            .extent = {800, 600},
            .name = "Setup Window",
            .fullScreenMode = FullScreenMode::eWindowed,
        };

        Window* window = Window::create(info, windowAPI);

        switch (windowAPI)
        {
            case WindowAPI::eGLFW:
            {
                auto* glfWindow = dynamicCast<GLFWindow*>(window);
                glfwCreateWindowSurface(_instance.instance,
                                        glfWindow->getGLFWWindow(),
                                        nullptr,
                                        &surface);
            }
            break;
            default:
                throw std::runtime_error("Unsupported window API");
        }

        assert(surface != VK_NULL_HANDLE);

        vkb::PhysicalDeviceSelector selector(_instance);
        selector.set_minimum_version(1, 3);
        selector.set_surface(surface);
        selector.add_required_extensions(
            {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
             VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
             VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
             VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME});
        selector.add_desired_extensions({VK_EXT_MESH_SHADER_EXTENSION_NAME});

        auto phys = selector.select();
        if (!phys)
        {
            throw std::runtime_error("Failed to select physical device");
        }
        _physicalDevice = phys.value();

        vkb::DeviceBuilder deviceBuilder(_physicalDevice);

        vk::PhysicalDeviceDescriptorIndexingFeatures
            descriptorIndexingFeatures {};
        descriptorIndexingFeatures.sType =
            vk::StructureType::ePhysicalDeviceDescriptorIndexingFeatures;
        descriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing =
            VK_TRUE;
        descriptorIndexingFeatures
            .shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
        descriptorIndexingFeatures
            .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
        descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing =
            VK_TRUE;
        descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing =
            VK_TRUE;
        descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing =
            VK_TRUE;
        descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing =
            VK_TRUE;
        descriptorIndexingFeatures
            .shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures
            .shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures
            .shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexingFeatures
            .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures
            .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending =
            VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount =
            VK_TRUE;
        descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures {};
        dynamicRenderingFeatures.sType =
            vk::StructureType::ePhysicalDeviceDynamicRenderingFeatures;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
        descriptorIndexingFeatures.setPNext(&dynamicRenderingFeatures);

        deviceBuilder.add_pNext(&descriptorIndexingFeatures);
        auto device = deviceBuilder.build();
        if (!device)
        {
            throw std::runtime_error("Failed to create Vulkan device");
        }
        _device = device.value();

        volkLoadDevice(_device.device);

        _graphicsQueue = _device.get_queue(vkb::QueueType::graphics).value();
        _presentQueue = _device.get_queue(vkb::QueueType::present).value();
        _computeQueue = _device.get_queue(vkb::QueueType::compute).value();

        vkb::destroy_surface(_instance, surface);
        delete window;
    }

    VulkanContext::~VulkanContext()
    {
        vkb::destroy_device(_device);
        vkb::destroy_instance(_instance);
    }

    auto VulkanContext::createSwapchain(Window& window) -> Swapchain*
    {
        return nullptr;
    }

    auto VulkanContext::getGraphicsQueue() const -> vk::Queue
    {
        return _graphicsQueue;
    }
    auto VulkanContext::getPresentQueue() const -> vk::Queue
    {
        return _presentQueue;
    }
    auto VulkanContext::getComputeQueue() const -> vk::Queue
    {
        return _computeQueue;
    }
    auto VulkanContext::getDevice() const -> vk::Device
    {
        return _device.device;
    }
    auto VulkanContext::getInstance() const -> vk::Instance
    {
        return _instance.instance;
    }
    auto VulkanContext::getPhysicalDevice() const -> vk::PhysicalDevice
    {
        return _physicalDevice.physical_device;
    }
}  // namespace exage::Graphics