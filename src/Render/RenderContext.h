#pragma once

#include "../pch.h"
#include "DynamicBufferPool.h"

namespace Engine {

    enum class PassType
    {
        None = 0,
        Normal,
        Shadow
    };

    struct RenderContext
    {
        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;

        DynamicBufferPool* InstancePool;
    };
}