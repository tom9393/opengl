#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class RigidBodyBindings {
    public:
        static void Register(sol::state& state);
    };

}
