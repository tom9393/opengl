#pragma once

#include <sol2/sol.hpp>

#include "pch.h"
#include "Scene/Components.h"
#include "Scene/Scene.h"

namespace Engine {

    class LuaBindings {
    public:
        static void Register(sol::state& state);

        static void BindComponents(Scene* scene);
        static void UnbindComponents(Scene* scene);
        static void ReloadScripts(Scene* scene);

    private:
        static void RegisterMath(sol::state& state);
        static void RegisterInputs(sol::state& state);
        static void RegisterComponents(sol::state& state);
        
        template <typename T>
        static void BindComponent(Entity &e);
    };

}