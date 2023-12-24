#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "exage/Projects/Serialization.h"

#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/meta/meta.hpp>
#include <entt/meta/resolve.hpp>
#include <exage/utils/serialization.h>

#include "cereal/archives/binary.hpp"
#include "exage/Projects/Level.h"
#include "exage/Renderer/Scene/Camera.h"
#include "exage/Renderer/Scene/Light.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/Scene/Hierarchy.h"

namespace exage::Projects
{
    namespace
    {
        template<typename T>
        auto serializeStorageWithCereal(
            entt::id_type id,
            const entt::sparse_set& storage,
            std::unordered_map<Entity, uint32_t>& entityToIndex) noexcept
            -> std::optional<ComponentData>
        {
            if (id == entt::type_hash<T, void>::value())
            {
                ComponentData componentData;

                for (Entity entity : storage)
                {
                    uint32_t index = entityToIndex[entity];

                    const void* data = storage.get(entity);
                    auto& component = *static_cast<const T*>(data);

                    try
                    {
                        std::stringstream ss {std::ios::out | std::ios::binary};
                        {
                            cereal::BinaryOutputArchive oarchive(ss);
                            oarchive(component);
                        }

                        componentData[index] = ss.str();
                    }

                    catch (const std::exception&)
                    {
                        componentData[index] = "";
                    }
                }

                return componentData;
            }

            return std::nullopt;
        };

        namespace
        {
            struct MappedComponentRelationship
            {
                uint32_t parent;

                uint32_t childCount;
                uint32_t firstChild;

                uint32_t nextSibling;
                uint32_t previousSibling;

                template<class Archive>
                void serialize(Archive& archive)
                {
                    archive(parent, childCount, firstChild, nextSibling, previousSibling);
                }
            };
        }  // namespace

        auto serializeStorage(entt::id_type id,
                              const entt::sparse_set& storage,
                              std::unordered_map<Entity, uint32_t>& entityToIndex) noexcept
            -> std::optional<std::pair<std::string, ComponentData>>
        {
            if (id == entt::type_hash<EntityRelationship, void>::value())
            {
                ComponentData componentData;

                for (Entity entity : storage)
                {
                    uint32_t index = entityToIndex[entity];

                    const void* data = storage.get(entity);
                    const auto& component = *static_cast<const EntityRelationship*>(data);
                    MappedComponentRelationship mappedComponent {};

                    if (entityToIndex.find(component.parent) != entityToIndex.end())
                    {
                        mappedComponent.parent = entityToIndex[component.parent];
                    }
                    else
                    {
                        mappedComponent.parent = entt::null;
                    }

                    if (entityToIndex.find(component.firstChild) != entityToIndex.end())
                    {
                        mappedComponent.firstChild = entityToIndex[component.firstChild];
                    }
                    else
                    {
                        mappedComponent.firstChild = entt::null;
                    }

                    if (entityToIndex.find(component.nextSibling) != entityToIndex.end())
                    {
                        mappedComponent.nextSibling = entityToIndex[component.nextSibling];
                    }
                    else
                    {
                        mappedComponent.nextSibling = entt::null;
                    }

                    if (entityToIndex.find(component.previousSibling) != entityToIndex.end())
                    {
                        mappedComponent.previousSibling = entityToIndex[component.previousSibling];
                    }
                    else
                    {
                        mappedComponent.previousSibling = entt::null;
                    }

                    mappedComponent.childCount = component.childCount;

                    try
                    {
                        std::stringstream ss {std::ios::out | std::ios::binary};
                        {
                            cereal::BinaryOutputArchive oarchive(ss);
                            oarchive(mappedComponent);
                        }

                        componentData[index] = ss.str();
                    }

                    catch (const std::exception&)
                    {
                        componentData[index] = "";
                    }
                }

                return std::pair {std::string {"MappedEntityRelationship"},
                                  std::move(componentData)};
            }

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

    [[nodiscard]] auto serializeScene(const Scene& scene) noexcept
        -> std::pair<uint32_t, std::unordered_map<std::string, ComponentData>>
    {
        auto& reg = scene.registry();

        std::unordered_map<Entity, uint32_t> entityToIndex;

        uint32_t entityCount = 0;
        reg.each(
            [&](Entity entity)
            {
                entityToIndex[entity] = entityCount;
                entityCount++;
            });

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

                std::stringstream ss {data, std::ios::in | std::ios::binary};

                try
                {
                    cereal::BinaryInputArchive iarchive(ss);

                    T component;
                    iarchive(component);

                    reg.emplace_or_replace<T>(entity, std::move(component));
                }
                catch (const std::exception&)
                {
                    reg.emplace_or_replace<T>(entity);
                }
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

            if (typeName == "MappedEntityRelationship")
            {
                for (const auto& [index, data] : data)
                {
                    Entity entity = indexToEntity.at(index);

                    std::stringstream ss {data, std::ios::in | std::ios::binary};

                    try
                    {
                        cereal::BinaryInputArchive iarchive(ss);

                        MappedComponentRelationship mappedComponent {};
                        iarchive(mappedComponent);

                        EntityRelationship component {};

                        if (mappedComponent.parent != entt::null)
                        {
                            component.parent = indexToEntity[mappedComponent.parent];
                        }

                        if (mappedComponent.firstChild != entt::null)
                        {
                            component.firstChild = indexToEntity[mappedComponent.firstChild];
                        }

                        if (mappedComponent.nextSibling != entt::null)
                        {
                            component.nextSibling = indexToEntity[mappedComponent.nextSibling];
                        }

                        if (mappedComponent.previousSibling != entt::null)
                        {
                            component.previousSibling =
                                indexToEntity[mappedComponent.previousSibling];
                        }

                        component.childCount = mappedComponent.childCount;

                        scene.registry().emplace_or_replace<EntityRelationship>(entity, component);
                    }

                    catch (const std::exception&)
                    {
                        scene.registry().emplace_or_replace<EntityRelationship>(entity);
                    }
                }

                continue;
            }

#define DESERIALIZE_STORAGE(T) \
    if (typeName == #T) \
    { \
        deserializeStorageWithCereal<T>(indexToEntity, data, scene.registry()); \
        continue; \
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