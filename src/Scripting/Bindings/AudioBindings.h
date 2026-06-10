#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class AudioBindings {
    public:
        static void Register(sol::state& state);
    };

}
