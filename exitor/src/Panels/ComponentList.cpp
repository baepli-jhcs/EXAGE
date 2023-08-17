#include "ComponentList.h"

#include <entt/core/type_info.hpp>

#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace exitor
{

    auto ComponentList::draw(exage::Scene& scene, exage::Entity selectedEntity) noexcept
        -> entt::id_type
    {
        ImGui::Begin("Components");

        if (!scene.isValid(selectedEntity))
        {
            ImGui::End();
            return entt::null;
        }

        auto& reg = scene.registry();

        bool isNull = _selectedTypeID == entt::null;
        auto contentRegion = isNull ? ImVec2(ImGui::GetContentRegionAvail().x, 0.0F)
                                    : ImVec2(ImGui::GetContentRegionAvail().x / 2.0F, 0.0F);
        if (ImGui::Button("Add Component", contentRegion))
        {
            ImGui::OpenPopup("Add Component");
        }

        if (!isNull)
        {
            ImGui::SameLine();

            if (ImGui::Button("Remove Component", contentRegion))
            {
                auto* storage = reg.storage(_selectedTypeID);
                if (storage != nullptr && storage->contains(selectedEntity))
                {
                    storage->remove(selectedEntity);
                }

                _selectedTypeID = entt::null;
            }
        }

        if (ImGui::BeginPopup("Add Component"))
        {
            if (ImGui::Selectable("3D Transform"))
            {
                reg.emplace_or_replace<exage::Transform3D>(selectedEntity);
            }

            if (ImGui::Selectable("Camera"))
            {
                reg.emplace_or_replace<exage::Renderer::Camera>(selectedEntity);
            }

            if (ImGui::Selectable("Static Mesh"))
            {
                reg.emplace_or_replace<exage::Renderer::StaticMeshComponent>(selectedEntity);
            }

            if (ImGui::Selectable("Directional Light"))
            {
                reg.emplace_or_replace<exage::Renderer::DirectionalLight>(selectedEntity);
            }

            if (ImGui::Selectable("Point Light"))
            {
                reg.emplace_or_replace<exage::Renderer::PointLight>(selectedEntity);
            }

            if (ImGui::Selectable("Spot Light"))
            {
                reg.emplace_or_replace<exage::Renderer::SpotLight>(selectedEntity);
            }

            if (ImGui::Selectable("Other"))
            {
                _componentAdderOpen = true;
            }

            ImGui::EndPopup();
        }

        for (auto&& curr : reg.storage())
        {
            entt::id_type id = curr.first;
            auto& storage = curr.second;
            if (!storage.contains(selectedEntity))
            {
                continue;
            }

            if (id == entt::type_hash<exage::Transform3D, void>::value())
            {
                bool selected = _selectedTypeID == id;
                if (ImGui::Selectable("3D Transform", selected))
                {
                    _selectedTypeID = id;
                }

                ImGui::Separator();
            }

            else if (id == entt::type_hash<exage::Renderer::Camera, void>::value())
            {
                bool selected = _selectedTypeID == id;
                if (ImGui::Selectable("Camera", selected))
                {
                    _selectedTypeID = id;
                }

                ImGui::Separator();
            }

            else if (id == entt::type_hash<exage::Renderer::StaticMeshComponent, void>::value())
            {
                bool selected = _selectedTypeID == id;
                if (ImGui::Selectable("Static Mesh", selected))
                {
                    _selectedTypeID = id;
                }

                ImGui::Separator();
            }
            // In future, will use reflection to get the name of the component for plugins

            drawComponentIfMatches<exage::Renderer::DirectionalLight>(id, "Directional Light");
            drawComponentIfMatches<exage::Renderer::PointLight>(id, "Point Light");
        }

        ImGui::End();

        if (_componentAdderOpen)
        {
            drawComponentAdder(scene, selectedEntity);
        }

        return _selectedTypeID;
    }

    template<typename Component>
    auto ComponentList::drawComponentIfMatches(entt::id_type id, const char* name) noexcept -> void
    {
        if (id == entt::type_hash<Component, void>::value())
        {
            bool selected = _selectedTypeID == id;
            if (ImGui::Selectable(name, selected))
            {
                _selectedTypeID = id;
            }

            ImGui::Separator();
        }
    }

    void ComponentList::drawComponentAdder(exage::Scene& scene,
                                           exage::Entity selectedEntity) noexcept
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(350, 400), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::Begin("Component Adder", &_componentAdderOpen);

        ImGui::Text("Add Component");
        ImGui::Separator();

        ImGui::BeginChild("Component Adder List", ImVec2(0, 300), true);

        auto& reg = scene.registry();

        if (ImGui::Selectable("3D Transform"))
        {
            reg.emplace_or_replace<exage::Transform3D>(selectedEntity);
            _componentAdderOpen = false;
        }

        if (ImGui::Selectable("Camera"))
        {
            reg.emplace_or_replace<exage::Renderer::Camera>(selectedEntity);
            _componentAdderOpen = false;
        }

        if (ImGui::Selectable("Static Mesh"))
        {
            reg.emplace_or_replace<exage::Renderer::StaticMeshComponent>(selectedEntity);
            _componentAdderOpen = false;
        }

        if (ImGui::Selectable("Directional Light"))
        {
            reg.emplace_or_replace<exage::Renderer::DirectionalLight>(selectedEntity);
            _componentAdderOpen = false;
        }

        if (ImGui::Selectable("Point Light"))
        {
            reg.emplace_or_replace<exage::Renderer::PointLight>(selectedEntity);
            _componentAdderOpen = false;
        }

        if (ImGui::Selectable("Spot Light"))
        {
            reg.emplace_or_replace<exage::Renderer::SpotLight>(selectedEntity);
            _componentAdderOpen = false;
        }

        ImGui::EndChild();

        // Component search bar
        ImGui::Separator();

        // Width of the search bar is the width of the window
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        ImGui::InputTextWithHint("##Component Search", "Search", &_componentAdderSearch);

        ImGui::End();
    }

}  // namespace exitor