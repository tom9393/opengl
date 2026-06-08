#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class InputBindings {
    public:
        static void Register(sol::state& state);
    };

}
