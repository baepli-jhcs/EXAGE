#include <array>

#include "AssetImport.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Filesystem/VFS.h"
#include "Stages/AssetImport.h"

namespace exitor
{
    constexpr static const char* OUTPUT_DIRECTORY_ID = "Select Directory";

    auto AssetImport::importAssetScreen(const std::filesystem::path& basePath,
                                        const exage::Projects::Project& project) noexcept
        -> std::optional<ModelImportDetails>
    {
        std::optional<ModelImportDetails> result;

        if (ImGui::BeginPopup(IMPORT_MODEL_ID, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Import Model");
            ImGui::Separator();

            ImGui::InputText("Model Path", &_modelPath);

            ImGui::SameLine();

            if (ImGui::Button("Select Model"))
            {
                _dialog.open("Select Model", _modelPath, {}, {});
            }

            if (_dialog.isReady())
            {
                _modelPath = _dialog.getResult();
                _dialog.clear();
            }

            ImGui::Separator();

            ImGui::InputText("Output Directory", &_outputDirectory);

            ImGui::SameLine();

            if (ImGui::Button("Select Directory"))
            {
                ImGui::OpenPopup(OUTPUT_DIRECTORY_ID);
            }

            VFS::openFolderPopup(
                OUTPUT_DIRECTORY_ID, basePath, project, _currentModelOutputPath, _outputDirectory);

            ImGui::Separator();

            ImGui::Checkbox("Import Into Scene", &_importIntoScene);

            ImGui::Separator();

            if (ImGui::Button("Import", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                result = ModelImportDetails {
                    std::move(_modelPath), std::move(_outputDirectory), _importIntoScene};

                _modelPath.clear();
                _outputDirectory.clear();
                _importIntoScene = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return result;
    }
}  // namespace exitor