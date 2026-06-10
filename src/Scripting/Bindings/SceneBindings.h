#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class SceneBindings {
    public:
        static void Register(sol::state& state);
    };

}
