#include "ScriptEngine.h"
#include "Core/Input.h"
#include "LuaBindings.h"
#include "Scene/Components.h"

namespace Engine {

    std::unique_ptr<sol::state> ScriptEngine::s_LuaState;
    Scene* ScriptEngine::s_Scene;
    std::vector<ScriptEngine::ScheduledCall> ScriptEngine::s_ScheduledCalls;
    float ScriptEngine::s_Elapsed = 0.0f;

    void ScriptEngine::Init()
    {
        s_LuaState = std::make_unique<sol::state>();
        s_LuaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package);

		LuaBindings::Register(*s_LuaState.get());
    }

    void ScriptEngine::Shutdown()
    {
        // Scheduled functions reference the Lua state; drop them before it dies.
        s_ScheduledCalls.clear();
        s_LuaState = nullptr;
        s_Scene = nullptr;
    }

    void ScriptEngine::Schedule(float seconds, sol::protected_function fn)
    {
        s_ScheduledCalls.push_back({ seconds, fn });
    }

    void ScriptEngine::RuntimeStart(Scene* scene)
    {
        s_Scene = scene;
        Init();
        s_Elapsed = 0.0f;

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
		s_Elapsed += ts.GetSeconds();
		(*s_LuaState)["Time"]["Delta"] = ts.GetSeconds();
		(*s_LuaState)["Time"]["Elapsed"] = s_Elapsed;

		// Run due Timer.After callbacks. Due entries are removed before invoking
		// so a callback can safely schedule new timers.
		std::vector<sol::protected_function> due;
		for (auto it = s_ScheduledCalls.begin(); it != s_ScheduledCalls.end();)
		{
			it->Remaining -= ts.GetSeconds();
			if (it->Remaining <= 0.0f)
			{
				due.push_back(it->Fn);
				it = s_ScheduledCalls.erase(it);
			}
			else ++it;
		}
		for (auto& fn : due)
		{
			auto result = fn();
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << "Timer.After error: " << err.what() << std::endl;
			}
		}

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