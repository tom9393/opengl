#pragma once

#include "pch.h"
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

        // True when the file is a serialized editor scene ("PSCN" magic), as opposed
        // to a baked model ("PMSC") sharing the .pmscene extension.
        static bool IsSceneFile(const std::string &path);

        private:
        Shared<Scene> m_Scene;
    };

};
