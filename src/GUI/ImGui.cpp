#include "exage/GUI/ImGui.h"

#include "ImGui.h"
#include "ImGuiPlatform/imgui_impl_glfw.h"
#include "ImGuiPlatform/imgui_impl_vulkan.h"
#include "exage/platform/GLFW/GLFWWindow.h"
#include "exage/platform/Vulkan/VulkanCommandBuffer.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::GUI
{
    ImGui::Instance::Instance(const InitInfo& initInfo) noexcept
        : _context(initInfo.context)
        , _api(initInfo.context.getAPI())
        , _windowAPI(initInfo.window.getAPI())
        , _imCtx(::ImGui::CreateContext())
    {
        ::ImGui::SetCurrentContext(_imCtx);

        ImGuiIO& io = ::ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
        io.ConfigFlags |=
            ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows

        ::ImGui::StyleColorsDark();

        ImGuiStyle& style = ::ImGui::GetStyle();
        if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
        {
            style.WindowRounding = 0.0F;
            style.Colors[ImGuiCol_WindowBg].w = 1.0F;
        }

        switch (_windowAPI)
        {
            case exage::System::API::eGLFW:
            {
                initGLFW(initInfo);
                break;
            }

            default:
                break;
        }

        switch (_api)
        {
            case exage::Graphics::API::eVulkan:
            {
                initVulkan(initInfo);
                break;
            }
            default:
                break;
        }

        buildFonts();
    }

    void ImGui::Instance::cleanup() noexcept
    {
        if (_imCtx == nullptr)
        {
            return;
        }

        ::ImGui::SetCurrentContext(_imCtx);

        switch (_api)
        {
            case exage::Graphics::API::eVulkan:
            {
                ImGui_ImplVulkan_Shutdown();
                break;
            }
            default:
                break;
        }

        switch (_windowAPI)
        {
            case exage::System::API::eGLFW:
            {
                ImGui_ImplGlfw_Shutdown();
                break;
            }
            default:
                break;
        }
    }

    ImGui::Instance::~Instance()
    {
        cleanup();
    }

    ImGui::Instance::Instance(Instance&& old) noexcept
        : _context(old._context)
        , _api(old._api)
        , _windowAPI(old._windowAPI)
        , _imCtx(old._imCtx)
    {
        old._imCtx = nullptr;
    }
    auto ImGui::Instance::operator=(Instance&& old) noexcept -> Instance&
    {
        if (this == &old)
        {
            return *this;
        }

        cleanup();

        _context = old._context;
        _api = old._api;
        _windowAPI = old._windowAPI;
        _imCtx = old._imCtx;
        old._imCtx = nullptr;
        return *this;
    }

    void ImGui::Instance::begin() noexcept
    {
        ::ImGui::SetCurrentContext(_imCtx);

        switch (_api)
        {
            case exage::Graphics::API::eVulkan:
            {
                ImGui_ImplVulkan_NewFrame();
                break;
            }
            default:
                break;
        }

        switch (_windowAPI)
        {
            case exage::System::API::eGLFW:
            {
                ImGui_ImplGlfw_NewFrame();
                break;
            }
            default:
                break;
        }

        ::ImGui::NewFrame();
    }

    void ImGui::Instance::end() noexcept
    {
        ::ImGui::SetCurrentContext(_imCtx);

        ::ImGui::Render();
    }
    void ImGui::Instance::addFont(const std::string& path, float size, bool isDefault) noexcept
    {
        ::ImGui::SetCurrentContext(_imCtx);

        ImGuiIO& io = ::ImGui::GetIO();
        ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size);
        if (isDefault)
        {
            io.FontDefault = font;
        }
    }
    void ImGui::Instance::buildFonts() noexcept
    {
        ::ImGui::SetCurrentContext(_imCtx);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void ImGui::Instance::renderMainWindow(exage::Graphics::CommandBuffer& commandBuffer) noexcept
    {
        ::ImGui::SetCurrentContext(_imCtx);

        switch (_api)
        {
            case exage::Graphics::API::eVulkan:
            {
                std::function const commandFunction =
                    [this](exage::Graphics::CommandBuffer& commandBuffer)
                {
                    vk::CommandBuffer const vkCommand =
                        commandBuffer.as<exage::Graphics::VulkanCommandBuffer>()
                            ->getCommandBuffer();
                    ImGui_ImplVulkan_RenderDrawData(::ImGui::GetDrawData(), vkCommand);
                };
                commandBuffer.userDefined(commandFunction);
                break;
            }
            default:
                break;
        }
    }
    void ImGui::Instance::renderAdditional()
    {
        ::ImGui::SetCurrentContext(_imCtx);

        ImGuiIO const& io = ::ImGui::GetIO();

        if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
        {
            ::ImGui::UpdatePlatformWindows();
            ::ImGui::RenderPlatformWindowsDefault();
        }
    }
    void ImGui::Instance::processEvent(const exage::System::Event& event) noexcept
    {
        switch (_windowAPI)
        {
            case exage::System::API::eGLFW:
            {
                ImGui_ImplGlfw_ProcessEvent(event);
                break;
            }
            default:
                break;
        }
    }
    void ImGui::Instance::initGLFW(const InitInfo& initInfo) noexcept
    {
        auto* window = initInfo.window.as<exage::System::GLFWWindow>();
        switch (_api)
        {
            case exage::Graphics::API::eVulkan:
                ImGui_ImplGlfw_InitForVulkan(window);
                break;
            default:
                break;
        }
    }
    void ImGui::Instance::initVulkan(const InitInfo& initInfo) noexcept
    {
        auto* context = initInfo.context.as<exage::Graphics::VulkanContext>();

        ImGui_ImplVulkan_InitInfo imInit = {};
        imInit.Instance = context->getInstance();
        imInit.PhysicalDevice = context->getPhysicalDevice();
        imInit.Device = context->getDevice();
        imInit.QueueFamily = context->getVulkanQueue().getFamilyIndex();
        imInit.Queue = context->getVulkanQueue().getVulkanQueue();
        imInit.PipelineCache = nullptr;
        imInit.Allocator = nullptr;
        imInit.MinImageCount = context->getVulkanQueue().getFramesInFlight();
        imInit.ImageCount = imInit.MinImageCount;
        imInit.ColorAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
        imInit.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&imInit);
    }
}  // namespace exage::GUI