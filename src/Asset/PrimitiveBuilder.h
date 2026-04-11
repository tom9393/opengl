#pragma once
#include "../../pch.h"
#include "Mesh.h"
#include "AssetManager.h"
#include <glm/glm.hpp>

namespace Engine {

    class PrimitiveBuilder
    {
    public:
        // These will generate the mesh and return the UUID registered in the AssetManager
        static UUID CreateCube();
        static UUID CreatePlane();
        static UUID CreateSphere(uint32_t xSegments = 32, uint32_t ySegments = 32);
    };

}