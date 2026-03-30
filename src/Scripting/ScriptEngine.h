#pragma once

#include <sol2/sol.hpp>

namespace Engine {

    class ScriptEngine {
    public:
        static void Init();
        static void Shutdown();
        static sol::state& GetState();

    private:
        static std::unique_ptr<sol::state> s_LuaState;
    };

}