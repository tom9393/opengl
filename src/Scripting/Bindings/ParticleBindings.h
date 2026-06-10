#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class ParticleBindings {
    public:
        static void Register(sol::state& state);
    };

}
