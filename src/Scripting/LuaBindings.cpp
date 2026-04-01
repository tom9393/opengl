#include "LuaBindings.h"
#include "ScriptEngine.h"
#include "../Core/Input.h"
#include "../Core/KeyCodes.h"
#include "../Scene/Components.h"
#include "../Physics/RigidBody.h"
#include <glm/glm.hpp>

namespace Engine {

    void LuaBindings::Register(sol::state& state) {
        RegisterMath(state);
        RegisterInputs(state);
        RegisterComponents(state);
    }

    void LuaBindings::RegisterMath(sol::state& state) {
        // Bind glm::vec3 with constructors and math operators
        state.new_usertype<glm::vec3>("vec3",
            sol::constructors<glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z,
            // Meta-functions allow Lua to use standard math operators (+, -, *, /)
            sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },

            // Overload to allow scalar multiplication
            sol::meta_function::multiplication, sol::overload(
            [](const glm::vec3& a, float b) { return a * b; },
            [](float a, const glm::vec3& b) { return a * b; }
            ),

            sol::meta_function::unary_minus, [](const glm::vec3& v) { return -v; }
        );
    }

    void LuaBindings::RegisterInputs(sol::state& state) {
        // Create a Lua table (namespace) for Input
        sol::table input = state.create_table("Input");
        
        // Bind Input functions
        input.set_function("IsKeyPressed", &Input::IsKeyPressed);
        input.set_function("IsMouseButtonPressed", &Input::IsMouseButtonPressed);
        input.set_function("GetMousePosition", &Input::GetMousePosition);

        // Bind Math Utility
        input.set_function("radians", [](float degrees) { return glm::radians(degrees); });

        // Expose some common KeyCodes as an enum
        state.new_enum("Key",
            "W", KEY_W,
            "A", KEY_A,
            "S", KEY_S,
            "D", KEY_D,
            "Space", KEY_SPACE,
            "Ctrl", KEY_LEFT_CONTROL,
            "Key1", KEY_1,
            "Key2", KEY_2,
            "Key3", KEY_3,
            "Key4", KEY_4,
            "Key5", KEY_5,
            "Key6", KEY_6,
            "Key7", KEY_7,
            "Key8", KEY_8,
            "Key9", KEY_9,
            "Key0", KEY_0
        );
    }

    void LuaBindings::RegisterComponents(sol::state& state)
    {

        // 1. Register the RaycastResult struct
        state.new_usertype<RaycastResult>("RaycastResult",
            "Hit",      &RaycastResult::Hit,
            "Entity",   &RaycastResult::HitEntity, // Sol2 handles the Entity* automatically
            "Position", &RaycastResult::Position,
            "Normal",   &RaycastResult::Normal
        );

        state.new_usertype<Entity>("Entity",
            "Destroy", [](Entity& e) { e.GetScene()->DestroyEntity(e); },
            "GetTransform", [](Entity& e) { return &e.GetComponent<TransformComponent>(); },
            "GetTag", [](Entity& e) { return &e.GetComponent<TagComponent>().tag; }
        );

        state.new_usertype<TransformComponent>("TransformComponent",
            "Translation", &TransformComponent::translation,
            "Rotation", &TransformComponent::rotation,
            "Scale", &TransformComponent::scale,
            "GetForward", &TransformComponent::GetForward,
            "GetRight", &TransformComponent::GetRight
        );

        state.new_usertype<Scene>("Scene",
        "CreateEntity", &Scene::CreateEntity,
        "Raycast", [](Scene& scene, const glm::vec3 origin, const glm::vec3 dir, float dist) {
            return PhysicsEngine::Raycast(origin, dir, dist);
        });

        state.new_usertype<TagComponent>("TagComponent",
            "Tag", &TagComponent::tag
        );

        state.new_usertype<RigidBody>("RigidBody",
            // --- Transformation ---
            "SetTransform", &RigidBody::SetTransform,
            "GetTransform", &RigidBody::GetTransform,
            
            // --- Forces & Impulses ---
            "ApplyForce", &RigidBody::ApplyForce,
            "ApplyImpulse", &RigidBody::ApplyImpulse,
            "ApplyTorque", &RigidBody::ApplyTorque,
            "ClearForces", &RigidBody::ClearForces,
            
            // --- Velocity ---
            "SetLinearVelocity", &RigidBody::SetLinearVelocity,
            "GetLinearVelocity", &RigidBody::GetLinearVelocity,
            "SetAngularVelocity", &RigidBody::SetAngularVelocity,
            
            // --- Physical Properties ---
            "SetMass", &RigidBody::SetMass,
            "SetDamping", &RigidBody::SetDamping,
            "SetRestitution", &RigidBody::SetRestitution,
            "SetFriction", &RigidBody::SetFriction,
            "SetGravity", &RigidBody::SetGravity,
            
            // --- Constraints & Modes ---
            "SetLinearFactor", &RigidBody::SetLinearFactor,
            "SetAngularFactor", &RigidBody::SetAngularFactor,
            "SetKinematic", &RigidBody::SetKinematic
        );

        state.new_usertype<SceneCameraComponent>("CameraComponent",
            "Primary", &SceneCameraComponent::primary,
            "FixedAspectRatio", &SceneCameraComponent::fixedAspectRatio
        );
    }

    void LuaBindings::ReloadScripts(Scene* scene)
    {
		auto& lua = ScriptEngine::GetState();
		auto view = scene->m_Registry.view<LuaScriptComponent>();

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
        // 1. Cast the void* to the actual type you registered (RigidBody)
        auto& rbComponent = e.GetComponent<RigidBodyComponent>();
        RigidBody* body = static_cast<RigidBody*>(rbComponent.runtimeBody);

        e.GetComponent<LuaScriptComponent>().Environment["RigidBody"] = body;
    }

    void LuaBindings::BindComponents(Scene* scene)
    {
		auto luaScripts = scene->m_Registry.view<LuaScriptComponent>();
        for (auto &e : luaScripts)
        {
            auto& lua = ScriptEngine::GetState();
            Entity entity = { e, scene };

            auto& script = entity.GetComponent<LuaScriptComponent>();
            
            // Create a unique environment for this entity
            script.Environment = sol::environment(lua, sol::create, lua.globals());

            script.Environment["Self"] = entity;
            script.Environment["Scene"] = scene;

            BindComponent<TagComponent>(entity);
            BindComponent<TransformComponent>(entity);
            BindComponent<SceneCameraComponent>(entity);
            BindComponent<RigidBodyComponent>(entity);

            // Load the script file into this specific environment
            lua.script_file(script.FilePath, script.Environment);

            // Call OnCreate if it exists
            sol::function onCreate = script.Environment["OnCreate"];
            if (onCreate.valid()) {
                onCreate();
            }
        }
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