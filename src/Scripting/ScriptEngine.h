#pragma once

#include <sol2/sol.hpp>

#include "Core/TimeStep.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"

namespace Engine {

    class ScriptEngine {
    public:
        static void Init();
        static void Shutdown();

        static void OnUpdate(TimeStep ts);

        static void RuntimeStart(Scene* scene);
        static void RuntimeStop();

        static void ReloadScripts();

        static void SetScene(Scene* scene) { s_Scene = scene; }
        static Scene* GetScene() { return s_Scene; }
        static sol::state& GetState() { return *s_LuaState.get(); }

    private:
        static std::unique_ptr<sol::state> s_LuaState;
        static Scene* s_Scene;
    };

}