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

        // Calls the entity's Lua OnDestroy, if its script defines one. No-op
        // outside play mode, where script environments are never bound.
        static void CallOnDestroy(Entity entity);

    private:
        template <typename T>
        static void BindComponent(Entity &e);
    };

}