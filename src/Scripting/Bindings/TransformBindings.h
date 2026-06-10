#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class TransformBindings {
    public:
        static void Register(sol::state& state);
    };

}
