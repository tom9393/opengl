#pragma once

#include "DrawCommand.h"

namespace Engine {

    struct RenderItems
    {
        std::vector<DrawCommand> Opaque;
        std::vector<DrawCommand> Shadows;
        std::vector<DrawCommand> Transparent;
    };

}