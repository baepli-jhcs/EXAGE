#pragma once

#include "entt/core/hashed_string.hpp"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    constexpr auto CURRENT_RENDER_DATA = entt::hashed_string {"CURRENT_RENDER_DATA"};
    constexpr auto LAST_RENDER_DATA = entt::hashed_string {"LAST_RENDER_DATA"};
}  // namespace exage::Renderer