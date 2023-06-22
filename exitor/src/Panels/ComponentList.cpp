#include "ComponentList.h"

#include <entt/core/type_info.hpp>

#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"
#include "imgui.h"

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

}  // namespace exitor