#include "exage/Renderer/GeometryPass/MeshSystem.h"

#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Mesh.h"

namespace exage::Renderer
{
    struct BindlessPushConstant
    {
        uint32_t cameraIndex;
        uint32_t transformIndex;
        uint32_t meshIndex;
        uint32_t materialIndex;
    };

    void MeshSystem::render(Graphics::CommandBuffer& commandBuffer, Scene& scene)
    {
        commandBuffer.bindPipeline(_pipeline);

        auto view = scene.registry().view<MeshComponent, Transform3D>();

        for (auto entity : view)
        {
            auto& meshComponent = view.get<MeshComponent>(entity);
            auto& transform = view.get<Transform3D>(entity);

            auto& cameraInfo =
                scene.getComponent<CameraRenderInfo>(getSceneCamera(scene));

            BindlessPushConstant pushConstant;
            pushConstant.cameraIndex = cameraInfo.bufferID->get().id;
            pushConstant.transformIndex = transform.bufferID->get().id;
        }
    }
}  // namespace exage::Renderer
