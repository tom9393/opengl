#pragma once
#include <sol2/sol.hpp>

namespace Engine {

    class LuaBindings {
    public:
        static void Register(sol::state& state);

    private:
        static void RegisterMath(sol::state& state);
        static void RegisterInputs(sol::state& state);
        static void RegisterComponents(sol::state& state);
    };

}