#pragma once

#include <filesystem>
#include <string>

#include <tl/expected.hpp>

#include "exage/Core/Core.h"
#include "exage/Projects/Level.h"
#include "exage/Scene/Scene.h"
#include "exage/utils/classes.h"

namespace exage::Projects
{
    struct Serializable  // Inherit a component from this class to make it serializable
    {
        Serializable() noexcept = default;
        virtual ~Serializable() = default;

        EXAGE_DEFAULT_COPY(Serializable);
        EXAGE_DEFAULT_MOVE(Serializable);

        [[nodiscard]] virtual auto serialize() const noexcept -> std::string = 0;
        virtual void deserialize(const std::string& data) noexcept = 0;
    };

    /* Returns a pair of entity count and component data */
    [[nodiscard]] auto serializeScene(Scene& scene) noexcept
        -> std::pair<uint32_t, std::unordered_map<std::string, ComponentData>>;

    [[nodiscard]] auto loadScene(
        uint32_t entityCount, const std::unordered_map<std::string, ComponentData>& componentData)
        -> Scene;

}  // namespace exage::Projects