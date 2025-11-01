#pragma once

#include "../pch.h"
#include "../include/Mesh.h"

namespace Engine
{

    class Model
    {
        public:
        Model(std::vector<Shared<Mesh>> &meshes, std::string &path) : m_Meshes(meshes), m_Directory(path) {}
        std::vector<Shared<Mesh>> GetMeshes() { return m_Meshes; };
        std::string GetPath() { return m_Directory; };

        private:
        std::vector<Shared<Mesh>> m_Meshes;
        std::string m_Directory;
    };

};
