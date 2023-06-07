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
        auto drawTransform3D(exage::Scene& scene, exage::Entity selectedEntity) noexcept -> void;
        auto drawCamera(exage::Scene& scene, exage::Entity selectedEntity) noexcept -> void;
        auto drawMesh(exage::Scene& scene, exage::Entity selectedEntity) noexcept -> void;
    };
}  // namespace exitor