#pragma once

#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct TransformEntry
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 prevModel;
        alignas(16) glm::mat4 modelViewProjection;
        alignas(16) glm::mat4 prevModelViewProjection;
    };

    struct CameraEntry
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
        alignas(16) glm::mat4 viewProjection;
        alignas(16) glm::mat4 prevView;
        alignas(16) glm::mat4 prevProj;
        alignas(16) glm::mat4 prevViewProjection;
    };

    struct PointLightEntry
    {
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 color;
        alignas(16) glm::vec4 physicalRadius;
        alignas(16) glm::vec4 attenuationRadius;
    };

    struct SceneData
    {
        entt::storage<Transform3D> currentTransforms;
        entt::storage<Transform3D> previousTransforms;

        entt::storage<Camera> currentCameras;
        entt::storage<Camera> previousCameras;

        entt::storage<StaticMeshComponent> currentStaticMeshes;
        entt::storage<StaticMeshComponent> previousStaticMeshes;

        entt::storage<PointLight> currentPointLights;
        entt::storage<PointLight> previousPointLights;

        entt::storage<SpotLight> currentSpotLights;
        entt::storage<SpotLight> previousSpotLights;

        entt::storage<DirectionalLight> currentDirectionalLights;
        entt::storage<DirectionalLight> previousDirectionalLights;

        std::optional<Graphics::ResizableDynamicBuffer> transformBuffer;
        std::optional<Graphics::ResizableDynamicBuffer> cameraBuffer;
        std::optional<Graphics::ResizableDynamicBuffer> pointLightBuffer;
    };

    struct CameraData
    {
        Entity cameraEntity;
        Camera camera;
    };

    /* Process: at start of frame, current scene data is swapped to become previous.
     * Then, current scene data is filled with new data. Next, scene renderer renders scene-specific
     * data. Next, camera renderer renders data relevant to a specific camera. */
}  // namespace exage::Renderer