#include "ComponentList.h"

#include <entt/core/type_info.hpp>

#include "exage/Renderer/Scene/Camera.h"
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

            ImGui::Separator();

            if (id == entt::type_id<exage::Transform3D>().index())
            {
                bool selected = _selectedTypeID == id;
                if (ImGui::Selectable("3D Transform", selected))
                {
                    _selectedTypeID = id;
                }
            }

            else if (id == entt::type_id<exage::Renderer::Camera>().index())
            {
                bool selected = _selectedTypeID == id;
                if (ImGui::Selectable("Camera", selected))
                {
                    _selectedTypeID = id;
                }
            }

            else if (id == entt::type_id<exage::Renderer::MeshComponent>().index())
            {
                bool selected = _selectedTypeID == id;
                if (ImGui::Selectable("Mesh", selected))
                {
                    _selectedTypeID = id;
                }
            }

            // In future, will use reflection to get the name of the component for plugins
        }

        ImGui::End();

        return _selectedTypeID;
    }

}  // namespace exitor