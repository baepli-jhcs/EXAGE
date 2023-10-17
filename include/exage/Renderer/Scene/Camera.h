#pragma once

#include <glm/glm.hpp>

#include "entt/entity/fwd.hpp"
#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct Camera
    {
        float fov = glm::radians(45.0F);
        float near = 0.1F;
        float far = 1000.0F;

        float exposure = 1.0F;
        float gamma = 2.2F;

        // Serialization
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(fov, near, far, exposure, gamma);
        }
    };

    struct CameraRenderInfo
    {
        struct Data
        {
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 projection;
            alignas(16) glm::mat4 viewProjection;
            alignas(16) glm::vec3 position;
        };

        Data data;
        std::optional<Graphics::DynamicFixedBuffer> buffer;
    };
}  // namespace exage::Renderer
