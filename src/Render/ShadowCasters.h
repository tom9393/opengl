#pragma once

#include "DrawCommand.h"

namespace Engine {

    struct ShadowCasterDirectional {
        glm::mat4 LightSpaceMatrix;
        std::vector<DrawCommand> RenderItems;
    };

    struct ShadowCasterCone {
        glm::mat4 LightSpaceMatrix;
        std::vector<DrawCommand> RenderItems;
    };

    struct ShadowCasterPoint {
        glm::vec3 Position;
        std::vector<DrawCommand> RenderItems;
    };

}