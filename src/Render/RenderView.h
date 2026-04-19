#pragma once

#include "DrawCommand.h"
#include "ShadowCasters.h"
#include "Light.h"

namespace Engine {

    struct CameraData
    {
        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;
        float padding;
    };

    struct RenderView {
        //CameraData Camera;
        std::vector<DrawCommand> OpaqueItems;
        std::vector<DrawCommand> TransparentItems;
        std::vector<ShadowCaster2D> DirectionalShadowCasters;
        std::vector<ShadowCasterPoint> PointShadowCasters;
        SceneLightData LightData;
    };

}