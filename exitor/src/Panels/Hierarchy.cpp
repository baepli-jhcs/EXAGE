#include "Hierarchy.h"

#include <fmt/format.h>

#include "exage/Core/Core.h"
#include "exage/Scene/Entity.h"
#include "imgui.h"

namespace exitor
{

    auto HierarchyPanel::draw(exage::Scene& scene) noexcept -> exage::Entity
    {
        ImGui::Begin("Scene Hierarchy");

        scene.forEachRoot([&](exage::Entity entity) { drawEntity(scene, entity); });

        ImGui::End();

        if (!scene.isValid(_selectedEntity))
        {
            _selectedEntity = entt::null;
        }

        return _selectedEntity;
    }

    void HierarchyPanel::drawEntity(exage::Scene& scene, exage::Entity entity) noexcept
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
        bool selected = ImGui::Selectable(name.c_str(), _selectedEntity == entity);

        if (selected)
        {
            _selectedEntity = entity;
        }

        if (opened)
        {
            scene.forEachChild(entity, [&](exage::Entity child) { drawEntity(scene, child); });

            ImGui::TreePop();
        }
    }

}  // namespace exitor