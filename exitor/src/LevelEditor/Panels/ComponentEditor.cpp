#include "ComponentEditor.h"

#include "exage/Core/Debug.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Rotation3D.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace exitor
{

    auto ComponentEditor::draw(exage::Scene& scene,
                               exage::Entity selectedEntity,
                               entt::id_type selectedTypeID,
                               exage::Projects::Project& project) noexcept -> void
    {
        ImGui::Begin("Component Editor");

        if (!scene.isValid(selectedEntity))
        {
            ImGui::End();
            return;
        }

        if (selectedTypeID == entt::type_hash<exage::Transform3D, void>::value())
        {
            drawTransform3D(scene, selectedEntity);
        }

        else if (selectedTypeID == entt::type_hash<exage::Renderer::Camera, void>::value())
        {
            drawCamera(scene, selectedEntity);
        }

        else if (selectedTypeID
                 == entt::type_hash<exage::Renderer::StaticMeshComponent, void>::value())
        {
            drawMesh(scene, selectedEntity, project);
        }

        else if (selectedTypeID
                 == entt::type_hash<exage::Renderer::DirectionalLight, void>::value())
        {
            drawDirectionalLight(scene, selectedEntity);
        }

        else if (selectedTypeID == entt::type_hash<exage::Renderer::PointLight, void>::value())
        {
            drawPointLight(scene, selectedEntity);
        }

        ImGui::End();
    }

    auto ComponentEditor::drawTransform3D(exage::Scene& scene,
                                          exage::Entity selectedEntity) noexcept -> void
    {
        if (!scene.hasComponent<exage::Transform3D>(selectedEntity))
        {
            return;
        }

        auto& transform = scene.getComponent<exage::Transform3D>(selectedEntity);

        ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);

        switch (transform.rotation.getRotationType())
        {
            case exage::RotationType::ePitchYawRoll:
            {
                std::optional<glm::vec3> angleResult = transform.rotation.getEuler();
                exage::debugAssume(angleResult.has_value(), "Failed to get euler angles");
                glm::vec3 degrees = glm::degrees(*angleResult);
                ImGui::DragFloat3("Pitch, Yaw, Roll", glm::value_ptr(degrees), 0.1f);
                transform.rotation =
                    exage::Rotation3D {glm::radians(degrees), exage::RotationType::ePitchYawRoll};
                break;
            }
            case exage::RotationType::eYawPitchRoll:
            {
                std::optional<glm::vec3> angleResult = transform.rotation.getEuler();
                exage::debugAssume(angleResult.has_value(), "Failed to get euler angles");
                glm::vec3 degrees = glm::degrees(*angleResult);
                ImGui::DragFloat3("Yaw, Pitch, Roll", glm::value_ptr(degrees), 0.1f);
                transform.rotation =
                    exage::Rotation3D {glm::radians(degrees), exage::RotationType::eYawPitchRoll};
                break;
            }
            case exage::RotationType::eQuaternion:
            {
                std::optional<glm::quat> quaternionResult = transform.rotation.getQuaternion();
                exage::debugAssume(quaternionResult.has_value(), "Failed to get quaternion");
                ImGui::DragFloat4("Quaternion", glm::value_ptr(quaternionResult.value()), 0.1f);
                transform.rotation = exage::Rotation3D {quaternionResult.value()};
                break;
            }

            default:
                break;
        }

        // Toggle between rotation types with dropdown
        const char* current = [&transform]() -> const char*
        {
            switch (transform.rotation.getRotationType())
            {
                case exage::RotationType::ePitchYawRoll:
                    return "Pitch, Yaw, Roll";
                case exage::RotationType::eYawPitchRoll:
                    return "Yaw, Pitch, Roll";
                case exage::RotationType::eQuaternion:
                    return "Quaternion";
                default:
                    return "Unknown";
            }
        }();

        if (ImGui::BeginCombo("##Rotation Type", current))
        {
            if (ImGui::Selectable("Pitch, Yaw, Roll"))
            {
                transform.rotation.setRotationType(exage::RotationType::ePitchYawRoll);
            }
            if (ImGui::Selectable("Yaw, Pitch, Roll"))
            {
                transform.rotation.setRotationType(exage::RotationType::eYawPitchRoll);
            }
            if (ImGui::Selectable("Quaternion"))
            {
                transform.rotation.setRotationType(exage::RotationType::eQuaternion);
            }
            ImGui::EndCombo();
        }

        ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f);

        ImGui::Text("Global Position: %f, %f, %f",
                    transform.globalPosition.x,
                    transform.globalPosition.y,
                    transform.globalPosition.z);

        glm::quat globalRotation = transform.globalRotation.getQuaternion();
        ImGui::Text("Global Rotation: %f, %f, %f, %f",
                    globalRotation.x,
                    globalRotation.y,
                    globalRotation.z,
                    globalRotation.w);

        ImGui::Text("Global Scale: %f, %f, %f",
                    transform.globalScale.x,
                    transform.globalScale.y,
                    transform.globalScale.z);
    }

    auto ComponentEditor::drawCamera(exage::Scene& scene, exage::Entity selectedEntity) noexcept
        -> void
    {
        if (!scene.hasComponent<exage::Renderer::Camera>(selectedEntity))
        {
            return;
        }

        auto& camera = scene.getComponent<exage::Renderer::Camera>(selectedEntity);

        // fov between 1 and 179 degrees
        auto fov = glm::degrees(camera.fov);
        ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 179.0f);
        camera.fov = glm::radians(fov);
        ImGui::DragFloat("Near", &camera.near, 0.1f);
        ImGui::DragFloat("Far", &camera.far, 0.1f);
    }

    auto ComponentEditor::drawMesh(exage::Scene& scene,
                                   exage::Entity selectedEntity,
                                   exage::Projects::Project& project) noexcept -> void
    {
        if (!scene.hasComponent<exage::Renderer::StaticMeshComponent>(selectedEntity))
        {
            return;
        }

        auto& meshComponent =
            scene.getComponent<exage::Renderer::StaticMeshComponent>(selectedEntity);

        ImGui::Text("Mesh: %s", meshComponent.path.c_str());

        // Selection dropdown for mesh
        if (ImGui::BeginCombo("##Mesh", meshComponent.path.c_str()))
        {
            for (auto& meshPath : project.meshPaths)
            {
                if (ImGui::Selectable(meshPath.c_str()))
                {
                    meshComponent.path = meshPath;

                    std::hash<std::string> hasher;
                    meshComponent.pathHash = hasher(meshPath);
                    if (_meshSelectionCallback)
                    {
                        _meshSelectionCallback(meshPath);
                    }
                }
            }
            ImGui::EndCombo();
        }
    }

    auto ComponentEditor::drawDirectionalLight(exage::Scene& scene,
                                               exage::Entity selectedEntity) noexcept -> void
    {
        if (!scene.hasComponent<exage::Renderer::DirectionalLight>(selectedEntity))
        {
            return;
        }

        auto& light = scene.getComponent<exage::Renderer::DirectionalLight>(selectedEntity);

        ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
        ImGui::DragFloat("Intensity", &light.intensity, 0.1f);
        ImGui::Checkbox("Cast Shadow", &light.castShadow);
    }

    void ComponentEditor::drawPointLight(exage::Scene& scene, exage::Entity selectedEntity) noexcept
    {
        if (!scene.hasComponent<exage::Renderer::PointLight>(selectedEntity))
        {
            return;
        }

        auto& light = scene.getComponent<exage::Renderer::PointLight>(selectedEntity);

        ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
        ImGui::DragFloat("Intensity", &light.intensity, 0.1f);
        ImGui::DragFloat("Physical Radius", &light.physicalRadius, 0.1f);
        ImGui::DragFloat("Attenuation Radius", &light.attenuationRadius, 0.1f);
        ImGui::Checkbox("Cast Shadow", &light.castShadow);
    }

}  // namespace exitor