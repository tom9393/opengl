#include "ScriptEngine.h"
#include "../Core/Input.h"
#include "../Scene/Components.h"
#include "LuaBindings.h"

namespace Engine {

    std::unique_ptr<sol::state> ScriptEngine::s_LuaState;
    Scene* ScriptEngine::s_Scene;

    void ScriptEngine::Init()
    {
        s_LuaState = std::make_unique<sol::state>();
        s_LuaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package);

		LuaBindings::Register(*s_LuaState.get());
    }

    void ScriptEngine::Shutdown()
    {
        s_LuaState = nullptr;
        s_Scene = nullptr;
    }

    void ScriptEngine::RuntimeStart(Scene* scene)
    {
        s_Scene = scene;
        Init();

		auto scripts = s_Scene->m_Registry.view<NativeScriptComponent>();
		for (auto& e : scripts)
		{
			Entity entity = { e, s_Scene };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.InstantiateScript(&nsc);
			nsc.instance->m_Entity = e;
			nsc.instance->m_Scene = s_Scene;
			nsc.instance->OnCreate();
		}

		LuaBindings::BindComponents(s_Scene);
    }

    void ScriptEngine::RuntimeStop()
    {
		auto scripts = s_Scene->m_Registry.view<NativeScriptComponent>();
		for (auto e : scripts)
		{
			Entity entity = { e, s_Scene };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.instance->OnDestroyed();
			nsc.DestroyScript(&nsc);
		}

		LuaBindings::UnbindComponents(s_Scene);

        Shutdown();
    }

    void ScriptEngine::ReloadScripts()
    {
		LuaBindings::ReloadScripts(s_Scene);
    }

    void ScriptEngine::OnUpdate(TimeStep ts)
    {
		auto scripts = s_Scene->m_Registry.view<NativeScriptComponent>();
		for (auto& e : scripts)
		{
			Entity entity = { e, s_Scene };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			if (nsc.instance)
				nsc.instance->OnUpdate(ts);
		}

		auto luaScripts = s_Scene->m_Registry.view<LuaScriptComponent>();
		for (auto entity : luaScripts)
		{
			auto& script = luaScripts.get<LuaScriptComponent>(entity);
			
			sol::function onUpdate = script.Environment["OnUpdate"];
			if (onUpdate.valid()) {
				onUpdate(ts.GetSeconds());
			}
		}
    }

}