#pragma once

#include "pch.h"

namespace Engine {

    // Shadow atlas capacities, shared by LightExtractor (which stops handing out
    // shadow indices past them) and the shadow passes (which size their layered
    // targets from them). Directional and cone casters share one 2D array.
    constexpr int MaxShadowCasters2D = 16;
    constexpr int MaxShadowCastersPoint = 8;

    // Far plane used both when rendering point-light shadow cubemaps and when
    // reconstructing distances from them in the deferred lighting shader.
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