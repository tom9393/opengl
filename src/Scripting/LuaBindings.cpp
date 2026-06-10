#include "LuaBindings.h"
#include "Bindings/MathBindings.h"
#include "Bindings/InputBindings.h"
#include "Bindings/TransformBindings.h"
#include "Bindings/TagBindings.h"
#include "Bindings/RigidBodyBindings.h"
#include "Bindings/CameraBindings.h"
#include "Bindings/LightBindings.h"
#include "Bindings/MaterialBindings.h"
#include "Bindings/EntityBindings.h"
#include "Bindings/SceneBindings.h"
#include "Bindings/AudioBindings.h"
#include "Asset/AssetManager.h"
#include "Physics/RigidBody.h"
#include "Scene/Components.h"
#include "ScriptEngine.h"

namespace Engine {

    void LuaBindings::Register(sol::state& state) {
        MathBindings::Register(state);
        InputBindings::Register(state);
        TransformBindings::Register(state);
        TagBindings::Register(state);
        RigidBodyBindings::Register(state);
        CameraBindings::Register(state);
        LightBindings::Register(state);
        MaterialBindings::Register(state);
        EntityBindings::Register(state);
        SceneBindings::Register(state);
        AudioBindings::Register(state);

        // Time.Delta / Time.Elapsed (seconds), refreshed by ScriptEngine each frame.
        sol::table time = state.create_table("Time");
        time["Delta"] = 0.0f;
        time["Elapsed"] = 0.0f;

        // Timer.After(seconds, fn) -> runs fn once after the delay (game time).
        sol::table timer = state.create_table("Timer");
        timer.set_function("After", &ScriptEngine::Schedule);

        sol::table log = state.create_table("Log");
        log.set_function("Info",  [](const std::string& msg) { std::cout << "[Info] "  << msg << std::endl; });
        log.set_function("Warn",  [](const std::string& msg) { std::cout << "[Warn] "  << msg << std::endl; });
        log.set_function("Error", [](const std::string& msg) { std::cerr << "[Error] " << msg << std::endl; });
    }

    void LuaBindings::ReloadScripts(Scene* scene)
    {
		auto& lua = ScriptEngine::GetState();
		auto view = scene->m_Registry.view<LuaScriptComponent>();

		for (auto entity : view) {
			auto& script = view.get<LuaScriptComponent>(entity);

            AssetMetadata scriptMetadata = AssetManager::GetMetadata(script.ScriptHandle);
            std::filesystem::path path = scriptMetadata.FilePath;

			if (script.Environment.valid() && !path.empty()) {
				// 1. Re-run the file into the EXISTING environment
				auto result = lua.script_file(path, script.Environment);
				
				if (result.valid()) {
					std::cout << "Reloaded: " << path << std::endl;
					
					// 2. Optionally re-call OnCreate to reset state
					sol::protected_function onCreate = script.Environment["OnCreate"];
					if (onCreate.valid()) {
						sol::set_environment(script.Environment, onCreate);
						onCreate();
					}
				} else {
					sol::error err = result;
					std::cerr << "Reload Error: " << err.what() << std::endl;
				}
			}
		}
    }

    template <>
    void LuaBindings::BindComponent<TagComponent>(Entity &e)
    {
        if (!e.HasComponent<TagComponent>()) return;
        e.GetComponent<LuaScriptComponent>().Environment["Tag"] = &e.GetComponent<TagComponent>().tag;
    }

    template <>
    void LuaBindings::BindComponent<TransformComponent>(Entity &e)
    {
        if (!e.HasComponent<TransformComponent>()) return;
        e.GetComponent<LuaScriptComponent>().Environment["Transform"] = &e.GetComponent<TransformComponent>();
    }

    template <>
    void LuaBindings::BindComponent<SceneCameraComponent>(Entity &e)
    {
        if (!e.HasComponent<SceneCameraComponent>()) return;
        e.GetComponent<LuaScriptComponent>().Environment["Camera"] = &e.GetComponent<SceneCameraComponent>();
    }

    template <>
    void LuaBindings::BindComponent<RigidBodyComponent>(Entity &e)
    {
        if (!e.HasComponent<RigidBodyComponent>()) return;
        auto& rbComponent = e.GetComponent<RigidBodyComponent>();
        e.GetComponent<LuaScriptComponent>().Environment["RigidBody"] = rbComponent.RuntimeBody;
    }

    template <>
    void LuaBindings::BindComponent<LightComponent>(Entity &e)
    {
        if (!e.HasComponent<LightComponent>()) return;
        e.GetComponent<LuaScriptComponent>().Environment["Light"] = &e.GetComponent<LightComponent>();
    }

    void LuaBindings::CallOnDestroy(Entity entity)
    {
        if (!entity.HasComponent<LuaScriptComponent>()) return;

        auto& env = entity.GetComponent<LuaScriptComponent>().Environment;
        if (!env.valid()) return;

        sol::protected_function onDestroy = env["OnDestroy"];
        if (!onDestroy.valid()) return;

        auto result = onDestroy();
        if (!result.valid())
        {
            sol::error err = result;
            std::cerr << "OnDestroy error: " << err.what() << std::endl;
        }
    }

    void LuaBindings::BindEntity(Entity entity)
    {
        auto& lua = ScriptEngine::GetState();
        auto& script = entity.GetComponent<LuaScriptComponent>();

        // Create a unique environment for this entity
        script.Environment = sol::environment(lua, sol::create, lua.globals());

        script.Environment["Self"] = entity;
        script.Environment["Scene"] = entity.GetScene();

        BindComponent<TagComponent>(entity);
        BindComponent<TransformComponent>(entity);
        BindComponent<SceneCameraComponent>(entity);
        BindComponent<RigidBodyComponent>(entity);
        BindComponent<LightComponent>(entity);

        AssetMetadata scriptMetadata = AssetManager::GetMetadata(script.ScriptHandle);
        std::filesystem::path path = scriptMetadata.FilePath;

        // Load the script file into this specific environment
        lua.script_file(path, script.Environment);

        // Call OnCreate if it exists
        sol::function onCreate = script.Environment["OnCreate"];
        if (onCreate.valid()) {
            onCreate();
        }
    }

    void LuaBindings::BindComponents(Scene* scene)
    {
		auto luaScripts = scene->m_Registry.view<LuaScriptComponent>();
        for (auto &e : luaScripts)
            BindEntity({ e, scene });
    }

    void LuaBindings::UnbindComponents(Scene* scene)
    {
		auto luaScripts = scene->m_Registry.view<LuaScriptComponent>();
		for (auto& e : luaScripts)
        {
			CallOnDestroy({ e, scene });

			auto& script = luaScripts.get<LuaScriptComponent>(e);
			script.Environment = sol::lua_nil;
		}
    }

}