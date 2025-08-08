#pragma once

#include "../pch.h"
#include "../include/mesh.h"

class Model
{
public:
    std::vector<std::shared_ptr<Mesh>> m_Meshes;
    std::string m_Directory;

    Model(std::vector<std::shared_ptr<Mesh>> &meshes) : m_Meshes(meshes) {}
    std::vector<std::shared_ptr<Mesh>> GetMeshes() { return m_Meshes; };
    
};

