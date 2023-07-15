#include "Hierarchy.h"

#include <fmt/format.h>

#include "exage/Core/Core.h"
#include "exage/Scene/Entity.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace exitor
{

    auto HierarchyPanel::draw(exage::Scene& scene, entt::entity ignored) noexcept -> exage::Entity
    {
        ImGui::Begin("Scene Hierarchy");

        // Plus icon
        if (ImGui::Button("+"))
        {
            // Create entity
            _selectedEntity = scene.createEntity();
        }

        ImGui::SameLine();

        bool disabled = false;
        if (!scene.isValid(_selectedEntity))
        {
            disabled = true;
            _selectedEntity = entt::null;
        }

        if (disabled)
        {
            // Style minus button as disabled
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }

        // Minus icon
        if (ImGui::Button("-"))
        {
            // Delete entity
            if (scene.isValid(_selectedEntity))
            {
                scene.destroyEntity(_selectedEntity);
                _selectedEntity = entt::null;
            }
        }

        if (disabled)
        {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        scene.forEachRoot(
            [&](exage::Entity entity)
            {
                if (entity != ignored)
                {
                    drawEntity(scene, ignored, entity);
                }
            });

        if (ImGui::BeginPopupContextWindow(
                "Hierarchy Context Menu",
                ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Entity"))
            {
                _selectedEntity = scene.createEntity();
            }

            ImGui::EndPopup();
        }

        ImGui::End();

        if (!scene.isValid(_selectedEntity))
        {
            _selectedEntity = entt::null;
        }

        return _selectedEntity;
    }

    void HierarchyPanel::drawEntity(exage::Scene& scene,
                                    exage::Entity ignored,
                                    exage::Entity entity) noexcept
    {
        const auto& relationship = scene.getComponent<exage::EntityRelationship>(entity);

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        std::string name = fmt::format("Entity {}", static_cast<uint32_t>(entity));

        if (relationship.childCount == 0)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        bool opened = ImGui::TreeNodeExV(
            reinterpret_cast<void*>(static_cast<intptr_t>(entity)), flags, "", nullptr);

        ImGui::SameLine();
        bool selected = ImGui::Selectable(
            name.c_str(), _selectedEntity == entity, ImGuiSelectableFlags_AllowDoubleClick);

        bool deleted = false;

        // check if right clicked
        if (ImGui::BeginPopupContextItem("Entity Context Menu", ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Create Child Entity"))
            {
                scene.createEntity(entity);
            }

            if (ImGui::MenuItem("Delete Entity"))
            {
                scene.destroyEntity(entity);
                deleted = true;
            }

            ImGui::EndPopup();
        }

        if (selected)
        {
            _selectedEntity = entity;
        }

        // If dragged onto
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
            {
                IM_ASSERT(payload->DataSize == sizeof(exage::Entity));

                exage::Entity droppedEntity = *reinterpret_cast<exage::Entity*>(payload->Data);

                if (droppedEntity != entity)
                {
                    scene.setParent(droppedEntity, entity);
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::EndDragDropTarget();
        }

        // If dragged from
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("Entity", &entity, sizeof(exage::Entity));

            ImGui::Text("Entity %d", static_cast<uint32_t>(entity));

            ImGui::EndDragDropSource();
        }

        if (opened && !deleted)
        {
            scene.forEachChild(entity,
                               [&](exage::Entity child)
                               {
                                   if (child != ignored)
                                   {
                                       drawEntity(scene, ignored, child);
                                   }
                               });

            ImGui::TreePop();
        }
    }

}  // namespace exitor