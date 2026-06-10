#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class MathBindings {
    public:
        static void Register(sol::state& state);
    };

}
