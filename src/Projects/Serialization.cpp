#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "exage/Projects/Serialization.h"

#include <cereal/archives/binary.hpp>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/meta/meta.hpp>
#include <entt/meta/resolve.hpp>

#include "exage/Projects/Level.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Renderer/Scene/Transform.h"

namespace exage::Projects
{
    namespace
    {
        template<typename T>
        auto serializeStorageWithCereal(
            entt::id_type id,
            entt::sparse_set& storage,
            std::unordered_map<Entity, uint32_t>& entityToIndex) noexcept
            -> std::optional<ComponentData>
        {
            if (id == entt::type_hash<T, void>::value())
            {
                ComponentData componentData;

                for (Entity entity : storage)
                {
                    uint32_t index = entityToIndex[entity];

                    void* data = storage.get(entity);
                    auto& component = *static_cast<T*>(data);

                    std::stringstream ss;
                    {
                        cereal::BinaryOutputArchive oarchive(ss);
                        oarchive(component);
                    }

                    componentData[index] = ss.str();
                }

                return componentData;
            }

            return std::nullopt;
        };

        auto serializeStorage(entt::id_type id,
                              entt::sparse_set& storage,
                              std::unordered_map<Entity, uint32_t>& entityToIndex) noexcept
            -> std::optional<std::pair<std::string, ComponentData>>
        {
#define SERIALIZE_STORAGE(T) \
    if (auto data = serializeStorageWithCereal<T>(id, storage, entityToIndex); data) \
    { \
        return std::pair {std::string {#T}, std::move(*data)}; \
    }

            SERIALIZE_STORAGE(exage::Transform3D);
            SERIALIZE_STORAGE(exage::Renderer::Camera);
            SERIALIZE_STORAGE(exage::Renderer::StaticMeshComponent);
            SERIALIZE_STORAGE(exage::Renderer::DirectionalLight);
            SERIALIZE_STORAGE(exage::Renderer::PointLight);
            SERIALIZE_STORAGE(exage::Renderer::SpotLight);

#undef SERIALIZE_STORAGE

#define DO_NOT_SERIALIZE_STORAGE(T) \
    if (id == entt::type_hash<T, void>::value()) \
    { \
        return std::nullopt; \
    }

            DO_NOT_SERIALIZE_STORAGE(exage::Renderer::DirectionalLightRenderInfo);
            DO_NOT_SERIALIZE_STORAGE(exage::Renderer::PointLightRenderInfo);
            DO_NOT_SERIALIZE_STORAGE(exage::Renderer::SpotLightRenderInfo);
            DO_NOT_SERIALIZE_STORAGE(exage::Renderer::CameraRenderInfo);
            DO_NOT_SERIALIZE_STORAGE(exage::Renderer::TransformRenderInfo);

#undef DO_NOT_SERIALIZE_STORAGE

            // Use reflection to get the name of the component for plugins, and if it inherits
            // Serializable then serialize it

            std::string_view enttName = storage.type().name();
            entt::meta_type type =
                entt::resolve(entt::hashed_string {enttName.data(), enttName.size()});

            auto typeNameProperty = type.prop(entt::hashed_string {"typeName"});
            if (!typeNameProperty)
            {
                return std::nullopt;
            }
            auto typeName = typeNameProperty.value().cast<std::string>();

            // TODO: Check if the component inherits Serializable. Use RTTR for this

            return std::pair {typeName, ComponentData {}};
        }
    }  // namespace

    [[nodiscard]] auto serializeScene(Scene& scene) noexcept
        -> std::pair<uint32_t, std::unordered_map<std::string, ComponentData>>
    {
        auto& reg = scene.registry();

        std::unordered_map<Entity, uint32_t> entityToIndex;

        uint32_t entityCount = 0;
        reg.each([&](Entity entity) { entityToIndex[entity] = entityCount++; });

        std::unordered_map<std::string, ComponentData> componentData;

        for (auto&& curr : reg.storage())
        {
            entt::id_type id = curr.first;
            auto& storage = curr.second;

            auto data = serializeStorage(id, storage, entityToIndex);
            if (data)
            {
                componentData[data->first] = std::move(data->second);
            }
        }

        return {entityCount, std::move(componentData)};
    }

    namespace
    {
        template<typename T>
        void deserializeStorageWithCereal(
            const std::unordered_map<uint32_t, Entity>& indexToEntity,
            const std::unordered_map<uint32_t, std::string>& componentData,
            entt::registry& reg)
        {
            for (const auto& [index, data] : componentData)
            {
                Entity entity = indexToEntity.at(index);

                std::stringstream ss {data};
                cereal::BinaryInputArchive iarchive(ss);

                T component;
                iarchive(component);

                reg.emplace_or_replace<T>(entity, std::move(component));
            }
        };

    }  // namespace

    auto loadScene(uint32_t entityCount,
                   const std::unordered_map<std::string, ComponentData>& componentData) -> Scene
    {
        Scene scene;

        scene.registry().reserve(entityCount);

        std::unordered_map<uint32_t, Entity> indexToEntity;

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            Entity entity = scene.registry().create();
            indexToEntity[i] = entity;
        }

        for (const auto& [typeName, data] : componentData)
        {
            // Now, we have to deserialize the data and add it to the registry

#define DESERIALIZE_STORAGE(T) \
    if (typeName == #T) \
    { \
        deserializeStorageWithCereal<T>(indexToEntity, data, scene.registry()); \
    }

            DESERIALIZE_STORAGE(exage::Transform3D);
            DESERIALIZE_STORAGE(exage::Renderer::Camera);
            DESERIALIZE_STORAGE(exage::Renderer::StaticMeshComponent);
            DESERIALIZE_STORAGE(exage::Renderer::DirectionalLight);
            DESERIALIZE_STORAGE(exage::Renderer::PointLight);
            DESERIALIZE_STORAGE(exage::Renderer::SpotLight);

#undef DESERIALIZE_STORAGE

#define DO_NOT_DESERIALIZE_STORAGE(T) \
    if (typeName == #T) \
    { \
        continue; \
    }

            DO_NOT_DESERIALIZE_STORAGE(exage::Renderer::DirectionalLightRenderInfo);
            DO_NOT_DESERIALIZE_STORAGE(exage::Renderer::PointLightRenderInfo);
            DO_NOT_DESERIALIZE_STORAGE(exage::Renderer::SpotLightRenderInfo);
            DO_NOT_DESERIALIZE_STORAGE(exage::Renderer::CameraRenderInfo);
            DO_NOT_DESERIALIZE_STORAGE(exage::Renderer::TransformRenderInfo);

#undef DO_NOT_DESERIALIZE_STORAGE

            // Use reflection to get the name of the component for plugins, and if it inherits
            // Serializable then default construct it and deserialize it
        }

        return scene;
    }

}  // namespace exage::Projects