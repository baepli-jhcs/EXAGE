#include <array>

#include "AssetImport.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Filesystem/VFS.h"
#include "Stages/AssetImport.h"
#include "utils/files.h"

namespace exitor
{
    constexpr static const char* OUTPUT_DIRECTORY_ID = "Select Directory";

    auto AssetImport::importAssetScreen(const std::filesystem::path& basePath,
                                        const exage::Projects::Project& project) noexcept
        -> std::pair<std::optional<ModelImportDetails>, std::optional<TextureImportDetails>>
    {
        std::optional<ModelImportDetails> modelImportDetails;
        std::optional<TextureImportDetails> textureImportDetails;

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

            ImGui::Checkbox("Optimize Texture Precision", &_optimizeTexturePrecision);

            ImGui::Separator();

            if (ImGui::Button("Import", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                modelImportDetails = ModelImportDetails {
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

        if (ImGui::BeginPopup(IMPORT_TEXTURE_ID, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Import Texture");
            ImGui::Separator();

            ImGui::InputText("Texture Path", &_texturePath);

            ImGui::SameLine();

            if (ImGui::Button("Select Texture"))
            {
                _dialog.open("Select Texture", _texturePath, {}, {});
            }

            if (_dialog.isReady())
            {
                _texturePath = _dialog.getResult();
                _dialog.clear();
            }

            ImGui::Separator();

            ImGui::InputText("Output Directory", &_outputTextureDirectory);

            ImGui::SameLine();

            if (ImGui::Button("Select Path"))
            {
                ImGui::OpenPopup(OUTPUT_DIRECTORY_ID);
            }

            VFS::openFolderPopup(OUTPUT_DIRECTORY_ID,
                                 basePath,
                                 project,
                                 _currentTextureOutputPath,
                                 _outputTextureDirectory);

            ImGui::InputText("Output Name", &_outputTextureName);

            ImGui::Separator();

            ImGui::Checkbox("Optimize Precision", &_optimizePrecision);

            ImGui::Separator();

            if (ImGui::Button("Import", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();

                std::string outputPath = appendFolder(_outputTextureDirectory, _outputTextureName);

                textureImportDetails = TextureImportDetails {
                    std::move(_texturePath), std::move(outputPath), _optimizePrecision};
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (modelImportDetails)
        {
            auto truePath = getTruePath(modelImportDetails->outputDirectory, basePath);
            if (!std::filesystem::exists(truePath))
            {
                ImGui::OpenPopup("Model Import Error");
                modelImportDetails.reset();
            }
        }

        if (textureImportDetails)
        {
            auto truePath = getTruePath(_outputTextureDirectory, basePath);
            if (!std::filesystem::exists(truePath))
            {
                ImGui::OpenPopup("Texture Import Error");
                textureImportDetails.reset();
            }
        }

        if (ImGui::BeginPopupModal(
                "Model Import Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Error: Output directory does not exist");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal(
                "Texture Import Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Error: Output directory does not exist");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return {std::move(modelImportDetails), std::move(textureImportDetails)};
    }
}  // namespace exitor