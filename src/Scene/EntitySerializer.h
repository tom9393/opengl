#pragma once

#include "pch.h"

namespace YAML { class Emitter; class Node; }

namespace Engine
{

    class Scene;
    class Entity;

    // Serializes/deserializes a single entity's components. Shared by SceneSerializer and PrefabSerializer.
    class EntitySerializer
    {
        public:
        // Writes one entity to the emitter. includeParent controls whether its parent link is written.
        static void SerializeEntity(YAML::Emitter& out, Entity e, bool includeParent = true);

        // Creates one entity (with components) from a node. Parent links are resolved by DeserializeEntities.
        static Entity DeserializeEntity(const Shared<Scene>& scene, YAML::Node& node);

        // Two-pass deserialization of a list of entity nodes: create every entity, then relink parents.
        static void DeserializeEntities(const Shared<Scene>& scene, YAML::Node entities);
    };

};
