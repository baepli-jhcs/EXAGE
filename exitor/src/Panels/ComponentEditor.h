#pragma once

#include "exage/Scene/Entity.h"
#include "exage/Scene/Scene.h"

namespace exitor
{
    class ComponentEditor
    {
      public:
        ComponentEditor() noexcept = default;
        ~ComponentEditor() = default;

        auto draw(exage::Scene& scene,
                  exage::Entity selectedEntity,
                  entt::id_type selectedTypeID) noexcept -> void;

      private:
        void drawTransform3D(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawCamera(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawMesh(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawDirectionalLight(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
        void drawPointLight(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
    };
}  // namespace exitor