#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class LightBindings {
    public:
        static void Register(sol::state& state);
    };

}
