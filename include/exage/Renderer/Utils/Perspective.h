#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

namespace exage::Renderer
{

    [[nodiscard]] inline auto perspectiveProject(
        float fov, float aspectRatio, float near, float far, bool depthZeroToOne)
    {
        if (depthZeroToOne)
        {
            return glm::perspectiveRH_ZO(fov, aspectRatio, near, far);
        }
        else
        {
            return glm::perspectiveRH_NO(fov, aspectRatio, near, far);
        }
    }
}  // namespace exage::Renderer