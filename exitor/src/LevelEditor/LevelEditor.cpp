#include "LevelEditor.h"

#include "imgui.h"

namespace exitor
{
    LevelEditor::LevelEditor(const LevelEditorCreateInfo& createInfo) noexcept
        : _fontManager(createInfo.fontManager)
        , _project(createInfo.project)
        , _projectPath(createInfo.projectPath)
        , _projectDirectory(createInfo.projectDirectory)
        , _level()
        , _levelPanel(*createInfo.context, *_fontManager, _level)
        , _hierarchyPanel()
        , _componentList()
        , _componentEditor()
    {
    }

    void LevelEditor::handleFonts() noexcept
    {
        _menuBarFont = _fontManager->getFont("Source Sans Pro Regular",
                                             static_cast<uint32_t>(16.0F * _dpiScale));
        _levelPanel.handleFonts();
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

        _dpiScale = viewport->DpiScale;
        ImGuiStyle& style = ImGui::GetStyle();
        style = ImGuiStyle();
        style.ScaleAllSizes(_dpiScale);

        ImGuiIO const& io = ImGui::GetIO();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
        ImGui::Begin("DockSpace", &open, windowFlags);
        ImGui::PopStyleVar();

        if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0)
        {
            ImGuiID const dockspaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), dockspaceFlags);
        }

        menuBar();
        _levelPanel.run(commandBuffer, deltaTime);
        Entity selectedEntity = _hierarchyPanel.draw(_level.scene, entt::null);
        entt::id_type componentID = _componentList.draw(_level.scene, selectedEntity);
        _componentEditor.draw(_level.scene, selectedEntity, componentID, _project);

        ImGui::End();
    }

    void LevelEditor::menuBar() noexcept
    {
        ImGui::PushFont(_menuBarFont);
        if (!ImGui::BeginMainMenuBar())
        {
            ImGui::PopFont();
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

        ImGui::EndMainMenuBar();
        ImGui::PopFont();
    }
}  // namespace exitor