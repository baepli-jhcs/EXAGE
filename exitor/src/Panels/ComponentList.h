#pragma once

#include "exage/Scene/Entity.h"
#include "exage/Scene/Scene.h"
#include "exage/utils/classes.h"

namespace exitor
{
    class ComponentList
    {
      public:
        ComponentList() noexcept = default;
        ~ComponentList() = default;

        EXAGE_DEFAULT_COPY(ComponentList);
        EXAGE_DEFAULT_MOVE(ComponentList);

        auto draw(exage::Scene& scene, exage::Entity selectedEntity) noexcept -> entt::id_type;

        void reset() noexcept
        {
            _selectedTypeID = entt::null;
            _componentAdderOpen = false;
            _componentAdderSearch.clear();
        }

      private:
        entt::id_type _selectedTypeID = entt::null;
        bool _componentAdderOpen = false;

        std::string _componentAdderSearch;

        template<typename Component>
        void drawComponentIfMatches(entt::id_type id, const char* name) noexcept;

        void drawComponentAdder(exage::Scene& scene, exage::Entity selectedEntity) noexcept;
    };
}  // namespace exitor