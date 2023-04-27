#include <execution>

#include "exage/Renderer/GeometryPass/MeshSystem.h"

#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/Transform.h"

namespace exage::Renderer
{
    auto aabbInFrustum(const AABB& aabb, const glm::mat4& modelViewProjection) noexcept -> bool
    {
        glm::vec4 corners[8] = {
            {aabb.min.x, aabb.min.y, aabb.min.z, 1.0},  // x y z
            {aabb.max.x, aabb.min.y, aabb.min.z, 1.0},  // X y z
            {aabb.min.x, aabb.max.y, aabb.min.z, 1.0},  // x Y z
            {aabb.max.x, aabb.max.y, aabb.min.z, 1.0},  // X Y z

            {aabb.min.x, aabb.min.y, aabb.max.z, 1.0},  // x y Z
            {aabb.max.x, aabb.min.y, aabb.max.z, 1.0},  // X y Z
            {aabb.min.x, aabb.max.y, aabb.max.z, 1.0},  // x Y Z
            {aabb.max.x, aabb.max.y, aabb.max.z, 1.0},  // X Y Z
        };

        bool inFrustum = false;

        for (const auto& corner : corners)
        {
            glm::vec4 transformed = modelViewProjection * corner;

            if (transformed.x >= -transformed.w && transformed.x <= transformed.w
                && transformed.y >= -transformed.w && transformed.y <= transformed.w
                && transformed.z >= -transformed.w && transformed.z <= transformed.w)
            {
                inFrustum = true;
                break;
            }
        }

        return inFrustum;
    }

    struct BindlessPushConstant
    {
        uint32_t cameraIndex;
        uint32_t transformIndex;
        uint32_t materialIndex;
    };

    void MeshSystem::render(Graphics::CommandBuffer& commandBuffer, Scene& scene)
    {
        commandBuffer.bindPipeline(_pipeline);

        auto view = scene.registry().view<GPUMesh, TransformRenderInfo>();

        auto func = [&](Entity entity)
        {
            const auto& mesh = view.get<GPUMesh>(entity);
            const auto& transform = view.get<TransformRenderInfo>(entity);

            const auto& cameraInfo = scene.getComponent<CameraRenderInfo>(getSceneCamera(scene));

            // Frustum culling using meshComponent.mesh.aabb
            if (!aabbInFrustum(mesh.aabb, transform.modelViewProjection))
            {
                return;
            }

            BindlessPushConstant pushConstant {};
            pushConstant.cameraIndex = cameraInfo.bufferID->get().id;
            pushConstant.transformIndex = transform.bufferID->get().id;

            size_t lodLevel = 0;  // TODO: LOD

            const auto& materialID = mesh.material.bufferID;
            commandBuffer.insertDataDependency(materialID);
            pushConstant.materialIndex = materialID->get().id;

            commandBuffer.setPushConstant(sizeof(pushConstant),
                                          reinterpret_cast<std::byte*>(&pushConstant));

            commandBuffer.drawIndexed(mesh.lods[lodLevel].indexCount,
                                      mesh.lods[lodLevel].indexOffset,
                                      mesh.lods[lodLevel].vertexOffset,
                                      1,
                                      0);
        };

        std::for_each(std::execution::parallel_policy(), view.begin(), view.end(), func);
    }
}  // namespace exage::Renderer
