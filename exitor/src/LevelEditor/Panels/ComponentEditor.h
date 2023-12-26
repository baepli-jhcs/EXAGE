#pragma once

#include <filesystem>

#include "exage/Projects/Project.h"
#include "exage/Scene/Entity.h"
#include "exage/Scene/Scene.h"

namespace exitor
{
    using SelectionCallback = std::function<void(const std::string&)>;

    class ComponentEditor
    {
      public:
        ComponentEditor() noexcept = default;
        ~ComponentEditor() = default;

        EXAGE_DEFAULT_COPY(ComponentEditor);
        EXAGE_DEFAULT_MOVE(ComponentEditor);

        auto draw(exage::Scene& scene,
                  exage::Entity selectedEntity,
                  entt::id_type selectedTypeID,
                  exage::Projects::Project& project) noexcept -> void;

        void setMeshSelectionCallback(SelectionCallback callback) noexcept
        {
            _meshSelectionCallback = std::move(callback);
        }

      private:
        void drawTransform3D(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawCamera(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawMesh(exage::Scene& scene,
                      exage::Entity selectedEntity,
                      exage::Projects::Project& project) noexcept;
        void drawDirectionalLight(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawPointLight(exage::Scene& scene, exage::Entity selectedEntity) noexcept;

        SelectionCallback _meshSelectionCallback;
    };
}  // namespace exitor