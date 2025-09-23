#pragma once

#include "../pch.h"
#include "../include/mesh.h"

class Model
{
    public:

    Model(std::vector<std::shared_ptr<Mesh>> &meshes, std::string &path) : m_Meshes(meshes), m_Directory(path) {}
    std::vector<Shared<Mesh>> GetMeshes() { return m_Meshes; };
    std::string GetPath() { return m_Directory; };

    private:
    std::vector<std::shared_ptr<Mesh>> m_Meshes;
    std::string m_Directory;
};

