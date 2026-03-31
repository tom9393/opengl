#include "ScriptEngine.h"
#include "../Core/Input.h"
#include "../Scene/Components.h"

namespace Engine {

    std::unique_ptr<sol::state> ScriptEngine::s_LuaState;
    Scene* ScriptEngine::s_Scene;

    void ScriptEngine::Init()
    {
        s_LuaState = std::make_unique<sol::state>();
        s_LuaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package);

        // TODO: Register C++ Engine API to Lua here
        // 1. Bind GLM Vectors
        s_LuaState->new_usertype<glm::vec3>("vec3",
            sol::constructors<glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z,
            sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
            sol::meta_function::multiplication, [](const glm::vec3& v, float s) { return v * s; }
        );

        // 2. Bind TransformComponent with methods
        s_LuaState->new_usertype<TransformComponent>("TransformComponent",
            "Translation", &TransformComponent::translation,
            "Rotation", &TransformComponent::rotation,
            "Scale", &TransformComponent::scale,
            "GetForward", &TransformComponent::GetForward,
            "GetRight", &TransformComponent::GetRight
        );

        // 3. Bind functions
        s_LuaState->set_function("IsKeyPressed", &Input::IsKeyPressed);
        s_LuaState->set_function("GetMousePosition", &Input::GetMousePosition);

        // 4. Bind Math Utility
        s_LuaState->set_function("radians", [](float degrees) { return glm::radians(degrees); });
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

		auto luaScripts = s_Scene->m_Registry.view<LuaScriptComponent>();

		for (auto entity : luaScripts) {
			auto& script = luaScripts.get<LuaScriptComponent>(entity);
			
            auto& lua = *s_LuaState.get();
            
            // Create a unique environment for this entity
            script.Environment = sol::environment(lua, sol::create, lua.globals());
            
            // Pass the entity's components into the Lua environment
            // (This allows Lua to say: `Transform.Translation.x = 5`)
            script.Environment["Transform"] = &s_Scene->m_Registry.get<TransformComponent>(entity);
            
            // Load the script file into this specific environment
            lua.script_file(script.FilePath, script.Environment);
            
            // Call OnCreate if it exists
            sol::function onCreate = script.Environment["OnCreate"];
            sol::function onClick = script.Environment["OnClick"];
            if (onCreate.valid()) {
                onCreate();
            }
		}

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

		auto luaScripts = s_Scene->m_Registry.view<LuaScriptComponent>();
		for (auto& e : luaScripts)
        {
			auto& script = luaScripts.get<LuaScriptComponent>(e);
			script.Environment = sol::lua_nil;
		}

        Shutdown();
    }

    void ScriptEngine::ReloadScripts()
    {
		auto& lua = *s_LuaState.get();
		auto view = s_Scene->m_Registry.view<LuaScriptComponent>();

		for (auto entity : view) {
			auto& script = view.get<LuaScriptComponent>(entity);

			if (script.Environment.valid() && !script.FilePath.empty()) {
				// 1. Re-run the file into the EXISTING environment
				auto result = lua.script_file(script.FilePath, script.Environment);
				
				if (result.valid()) {
					std::cout << "Reloaded: " << script.FilePath << std::endl;
					
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
				onUpdate(ts.GetSeconds()); // Pass delta time to Lua
			} else {
				std::cout << "Something went wrong" << std::endl;
			}
		}
    }

}