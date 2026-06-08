#include "LuaBindings.h"
#include "MathBindings.h"
#include "InputBindings.h"
#include "ComponentBindings.h"
#include "Asset/AssetManager.h"
#include "Physics/RigidBody.h"
#include "Scene/Components.h"
#include "ScriptEngine.h"

namespace Engine {

    void LuaBindings::Register(sol::state& state) {
        MathBindings::Register(state);
        InputBindings::Register(state);
        ComponentBindings::Register(state);
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
			auto& script = luaScripts.get<LuaScriptComponent>(e);
			script.Environment = sol::lua_nil;
		}
    }

}