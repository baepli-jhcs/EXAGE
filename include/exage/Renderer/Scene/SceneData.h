#pragma once

#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/Transform.h"
#include "exage/Scene/Scene.h"

namespace exage::Renderer
{
    struct SceneData
    {
        entt::storage<Camera> currentCameras;
        entt::storage<Camera> previousCameras;

        entt::storage<Transform3D> currentTransforms;
        entt::storage<Transform3D> previousTransforms;

        entt::storage<StaticMeshComponent> currentStaticMeshes;
        entt::storage<StaticMeshComponent> previousStaticMeshes;

        entt::storage<PointLight> currentPointLights;
        entt::storage<PointLight> previousPointLights;

        entt::storage<DirectionalLight> currentDirectionalLights;
        entt::storage<DirectionalLight> previousDirectionalLights;

        entt::storage<SpotLight> currentSpotLights;
        entt::storage<SpotLight> previousSpotLights;

        entt::storage<TransformRenderInfo> transformRenderInfo;
        entt::storage<PointLightRenderInfo> pointLightRenderInfo;
        entt::storage<SpotLightRenderInfo> spotLightRenderInfo;
    };

    struct CameraData
    {
        /* Directional lights are rendered using cascaded shadow maps and depend on the camera.
         * Therefore, they are stored in the camera data. */
        entt::storage<DirectionalLightRenderInfo> directionalLightRenderInfo;
    };

    /* Process: at start of frame, current scene data is swapped to become previous.
     * Then, current scene data is filled with new data. Next, scene renderer renders scene-specific
     * data. Next, camera renderer renders data relevant to a specific camera. */
}  // namespace exage::Renderer