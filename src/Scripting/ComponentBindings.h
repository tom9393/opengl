#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class ComponentBindings {
    public:
        static void Register(sol::state& state);
    };

}
