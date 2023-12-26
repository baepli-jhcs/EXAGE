#include "LevelEditor.h"

#include "imgui.h"

namespace exitor
{
    LevelEditor::LevelEditor(const LevelEditorCreateInfo& createInfo) noexcept
        : _project(createInfo.project)
        , _projectPath(createInfo.projectPath)
        , _projectDirectory(createInfo.projectDirectory)
        , _level()
        , _levelPanel(*createInfo.context, _level)
    {
    }

    void LevelEditor::run(Graphics::CommandBuffer& commandBuffer, float deltaTime) noexcept
    {
        bool open = true;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus;

        static ImGuiDockNodeFlags const dockspaceFlags = ImGuiDockNodeFlags_None;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiIO const& io = ImGui::GetIO();
        ImGuiStyle const& style = ImGui::GetStyle();

        ImGui::Begin("DockSpace", &open, windowFlags);

        if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0)
        {
            ImGuiID const dockspaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), dockspaceFlags);
        }

        menuBar();
        _levelPanel.run(commandBuffer, deltaTime);

        ImGui::End();
    }

    void LevelEditor::menuBar() noexcept
    {
        if (!ImGui::BeginMenuBar())
        {
            return;
        }

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Level"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Open Level"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save Level"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}  // namespace exitor