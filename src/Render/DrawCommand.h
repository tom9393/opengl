#pragma once

#include "pch.h"

namespace Engine {

    class Mesh;
    class Material;

    struct DrawCommand
    {
        Shared<Mesh> TargetMesh;
        Shared<Material> TargetMaterial;
        glm::mat4 Transform;
    };
}