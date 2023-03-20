﻿#include "Graphics/HLPD/ImGuiTools.h"

#include <Vulkan/VulkanCommandBuffer.h>

#include "GLFW/GLFWindow.h"
#include "ImGuiPlatform/imgui_impl_glfw.h"
#include "ImGuiPlatform/imgui_impl_vulkan.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    ImGuiInstance::ImGuiInstance(const ImGuiInitInfo& initInfo) noexcept
        : _context(initInfo.context)
        , _queue(initInfo.queue)
    {
        _api = initInfo.context.getAPI();
        _windowAPI = initInfo.window.getAPI();

        IMGUI_CHECKVERSION();
        _imCtx = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
        io.ConfigFlags |=
            ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        switch (_windowAPI)
        {
            case WindowAPI::eGLFW:
            {
                initGLFW(initInfo);
                break;
            }

            default:
                break;
        }

        switch (_api)
        {
            case API::eVulkan:
            {
                initVulkan(initInfo);
                break;
            }
            default:
                break;
        }

        buildFonts();
    }

    ImGuiInstance::~ImGuiInstance()
    {
        switch (_api)
        {
			case API::eVulkan:
            {
				ImGui_ImplVulkan_Shutdown();
				break;
			}
			default:
				break;
		}

        switch (_windowAPI)
        {
            case WindowAPI::eGLFW:
            {
				ImGui_ImplGlfw_Shutdown();
				break;
			}
			default:
				break;
        }

        ImGui::DestroyContext(_imCtx);
    }

    void ImGuiInstance::begin() noexcept
    {
        ImGui::SetCurrentContext(_imCtx);

        switch (_api)
        {
            case API::eVulkan:
            {
                ImGui_ImplVulkan_NewFrame();
                break;
            }
            default:
                break;
        }

        switch (_windowAPI)
        {
			case WindowAPI::eGLFW:
            {
				ImGui_ImplGlfw_NewFrame();
				break;
			}
			default:
				break;
		}

        ImGui::NewFrame();
    }

    void ImGuiInstance::end() noexcept
    {
        ImGui::SetCurrentContext(_imCtx);
        ImGui::Render();
    }

    void ImGuiInstance::addFont(const std::string& path, float size, bool isDefault) noexcept
    {
        ImGui::SetCurrentContext(_imCtx);

        ImGuiIO& io = ImGui::GetIO();
        ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size);
        if (isDefault)
        {
            io.FontDefault = font;
        }

        buildFonts();
    }

    void ImGuiInstance::buildFonts() noexcept
    {
        std::unique_ptr<CommandBuffer> commandBuffer = _context.get().createCommandBuffer();
        commandBuffer->begin();

        switch (_api)
        {
            case API::eVulkan:
            {
                std::function commandFunction = [this](CommandBuffer& cmd)
                {
                    ImGui::SetCurrentContext(_imCtx);
                    vk::CommandBuffer vkCommand = cmd.as<VulkanCommandBuffer>()->getCommandBuffer();
                    ImGui_ImplVulkan_CreateFontsTexture(vkCommand);
                };
                commandBuffer->userDefined(commandFunction);
            }
            break;
            default:
                break;
        }
        
        commandBuffer->end();
        _queue.get().submitTemporary(std::move(commandBuffer));

        switch (_api)
        {
            case API::eVulkan:
            {
                ImGui_ImplVulkan_DestroyFontUploadObjects();
                break;
            }
            default:
                break;
        }

        _context.get().waitIdle();
    }

    void ImGuiInstance::renderMainWindow(CommandBuffer& commandBuffer) noexcept
    {
        switch (_api)
        {
            case API::eVulkan:
            {
                std::function commandFunction = [this](CommandBuffer& commandBuffer)
                {
                    ImGui::SetCurrentContext(_imCtx);
                    vk::CommandBuffer vkCommand =
                        commandBuffer.as<VulkanCommandBuffer>()->getCommandBuffer();
                    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCommand);
                };
                commandBuffer.userDefined(commandFunction);
                break;
            }
            default:
                break;
        }
    }

    void ImGuiInstance::renderAdditional()
    {
        ImGui::SetCurrentContext(_imCtx);

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImGuiInstance::initGLFW(const ImGuiInitInfo& initInfo) noexcept
    {
        auto* window = initInfo.window.as<GLFWindow>();
        switch (_api)
        {
            case API::eVulkan:
                ImGui_ImplGlfw_InitForVulkan(window->getGLFWWindow(), true);
                break;
            default:
                break;
        }
    }

    void ImGuiInstance::initVulkan(const ImGuiInitInfo& initInfo) noexcept
    {
        auto* context = initInfo.context.as<VulkanContext>();
        auto* window = initInfo.window.as<GLFWindow>();

        ImGui_ImplVulkan_InitInfo imInit = {};
        imInit.Instance = context->getInstance();
        imInit.PhysicalDevice = context->getPhysicalDevice();
        imInit.Device = context->getDevice();
        imInit.QueueFamily = context->getQueueIndex();
        imInit.Queue = context->getVulkanQueue();
        imInit.PipelineCache = nullptr;
        imInit.Allocator = nullptr;
        imInit.MinImageCount = initInfo.maxImageCount;
        imInit.ImageCount = initInfo.maxImageCount;
        imInit.ColorAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
        imInit.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&imInit);
    }
}  // namespace exage::Graphics
