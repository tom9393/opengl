#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class MaterialBindings {
    public:
        static void Register(sol::state& state);
    };

}
