#pragma once

#include "pch.h"
#include "Scene.h"

namespace Engine
{

    class Scene;
    class Entity;

    class PrefabSerializer
    {
        public:
        PrefabSerializer(const Shared<Scene> &scene);

        bool Deserialize(const std::string &path);

        // Writes an entity + its descendants to a .prefab file (component data + asset handle references)
        static void Serialize(Entity root, const std::string &path);

        private:
        Shared<Scene> m_Scene;
    };

};
