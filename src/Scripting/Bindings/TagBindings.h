#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class TagBindings {
    public:
        static void Register(sol::state& state);
    };

}
