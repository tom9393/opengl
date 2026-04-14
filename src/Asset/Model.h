#pragma once

#include "../pch.h"
#include "Asset.h"
#include "Mesh.h"

namespace Engine
{

    struct ModelMaterial
    {
        std::string Name;
        std::string Path;
        unsigned int Index;
        UUID Handle;
    };

    struct ModelMesh
    {
        std::string Name;
        std::string Path;
        unsigned int Index;
        UUID Handle;
        std::vector<ModelMaterial> Materials;
    };

    struct Model : public Asset
    {
        std::string Name;
        std::vector<ModelMesh> Meshes;
    };

};
