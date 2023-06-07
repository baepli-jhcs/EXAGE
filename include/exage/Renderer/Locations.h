#pragma once

#include "entt/core/hashed_string.hpp"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    const auto CURRENT_TRANSFORM_3D = entt::hashed_string {"EXAGE_CURRENT_TRANSFORM_3D"};
    const auto LAST_TRANSFORM_3D = entt::hashed_string {"EXAGE_LAST_TRANSFORM_3D"};

    const auto CURRENT_MESH_COMPONENT = entt::hashed_string {"EXAGE_CURRENT_GPU_MESH"};
    const auto LAST_MESH_COMPONENT = entt::hashed_string {"EXAGE_LAST_GPU_MESH"};

    const auto CURRENT_CAMERA = entt::hashed_string {"EXAGE_CURRENT_CAMERA"};
    const auto LAST_CAMERA = entt::hashed_string {"EXAGE_LAST_CAMERA"};

    const auto CURRENT_CAMERA_RENDER_INFO =
        entt::hashed_string {"EXAGE_CURRENT_CAMERA_RENDER_INFO"};
    const auto CURRENT_TRANSFORM_RENDER_INFO =
        entt::hashed_string {"EXAGE_CURRENT_TRANSFORM_RENDER_INFO"};
}  // namespace exage::Renderer