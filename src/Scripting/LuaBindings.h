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
        static void BindEntity(Entity entity);
        static void UnbindComponents(Scene* scene);
        static void ReloadScripts(Scene* scene);

    private:
        template <typename T>
        static void BindComponent(Entity &e);
    };

}