#pragma once

#include "../../pch.h"
#include "Scene.h"

namespace Engine
{

    class Scene;

    class SceneSerializer
    {
        public:
        SceneSerializer(const Shared<Scene> &scene);

        void Serialize(const std::string &path);
        bool Deserialize(const std::string &path);

        private:
        Shared<Scene> m_Scene;
    };

};
