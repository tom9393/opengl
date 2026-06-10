#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class CameraBindings {
    public:
        static void Register(sol::state& state);
    };

}
