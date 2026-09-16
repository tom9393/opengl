#pragma once

#include "pch.h"

namespace Engine {

    constexpr int MaxShadowCasters2D = 16;
    constexpr int MaxShadowCastersPoint = 8;

    constexpr float PointShadowFarPlane = 35.0f;

    struct GPULight {
        glm::vec4 PositionType;   // xyz = position, w = type (0=Point, 1=Directional, 2=Cone)
        glm::vec4 ColorInner;     // xyz = color, w = inner cutoff
        glm::vec4 DirectionOuter; // xyz = direction, w = outer cutoff
        glm::mat4 LightSpaceMatrix;
        glm::ivec4 Params; // x = shadowindex, yzw padding
    };

    struct SceneLightData {
        GPULight Lights[64];
        int LightCount = 0;
    };

}