#pragma once

#include "exage/Scene/Entity.h"
#include "exage/Scene/Scene.h"
#include "exage/utils/classes.h"

namespace exitor
{
    class HierarchyPanel
    {
      public:
        HierarchyPanel() noexcept = default;
        ~HierarchyPanel() = default;

        EXAGE_DEFAULT_COPY(HierarchyPanel);
        EXAGE_DEFAULT_MOVE(HierarchyPanel);

        auto draw(exage::Scene& scene, exage::Entity ignored = entt::null) noexcept
            -> exage::Entity;

      private:
        void drawEntity(exage::Scene& scene, exage::Entity ignored, exage::Entity entity) noexcept;

        exage::Entity _selectedEntity = entt::null;
    };
}  // namespace exitor