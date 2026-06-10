#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class EntityBindings {
    public:
        static void Register(sol::state& state);
    };

}
