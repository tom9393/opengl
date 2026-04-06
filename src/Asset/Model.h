#pragma once

#include "../pch.h"
#include "Asset.h"
#include "Mesh.h"

namespace Engine
{

    struct Model : public Asset
    {
        std::vector<UUID> Meshes;
        std::vector<UUID> Materials;
        std::vector<uint32_t> MeshMaterialIndices;
    };

};
