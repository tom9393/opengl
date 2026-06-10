#include "EntityBindings.h"

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

namespace Engine {

    void EntityBindings::Register(sol::state& state)
    {
        state.new_usertype<Entity>("Entity",
            "Destroy", [](Entity& e) { e.GetScene()->DestroyEntity(e); },
            // False once the entity has been destroyed (or was never found).
            "IsValid", [](Entity& e) {
                return (bool)e && e.GetScene() && e.GetScene()->GetRegistry().valid(e.GetHandle());
            },
            "GetTag", [](Entity& e) -> std::string {
                return e.HasComponent<TagComponent>() ? e.GetComponent<TagComponent>().tag : std::string{};
            },
            "GetLuaEnvironment", [](Entity& e) -> sol::object {
                if (e.HasComponent<LuaScriptComponent>()) {
                    auto& script = e.GetComponent<LuaScriptComponent>();

                    if (script.Environment.valid()) {
                        return script.Environment;
                    }
                }
                return sol::nil;
            },
            // Entity:Send("FunctionName", ...) calls that function in the target's
            // script environment, passing the extra arguments along. Returns the
            // function's result, or nil if the target has no such script function.
            "Send", [](Entity& e, const std::string& name, sol::variadic_args args, sol::this_state s) -> sol::object {
                if (!e.HasComponent<LuaScriptComponent>()) return sol::nil;

                auto& env = e.GetComponent<LuaScriptComponent>().Environment;
                if (!env.valid()) return sol::nil;

                sol::protected_function fn = env[name];
                if (!fn.valid()) return sol::nil;

                auto result = fn(args);
                if (!result.valid()) {
                    sol::error err = result;
                    std::cerr << "Entity.Send(" << name << "): " << err.what() << std::endl;
                    return sol::nil;
                }
                return result.return_count() > 0 ? result.get<sol::object>() : sol::nil;
            },
            "GetParent", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasParent()) return sol::nil;
                return sol::make_object(s, Entity{ e.GetParent(), e.GetScene() });
            },
            "GetChildren", [](Entity& e) {
                std::vector<Entity> children;
                for (auto handle : e.GetChildren())
                    children.push_back(Entity{ handle, e.GetScene() });
                return children;
            },
            // First immediate child whose tag matches, or nil.
            "GetChild", [](Entity& e, const std::string& name, sol::this_state s) -> sol::object {
                for (auto handle : e.GetChildren()) {
                    Entity child{ handle, e.GetScene() };
                    if (child.HasComponent<TagComponent>() && child.GetComponent<TagComponent>().tag == name)
                        return sol::make_object(s, child);
                }
                return sol::nil;
            },
            // Component access on arbitrary entities (e.g. a raycast hit or a
            // Scene:Find result). Each returns nil if the component is missing.
            "GetTransform", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasComponent<TransformComponent>()) return sol::nil;
                return sol::make_object(s, &e.GetComponent<TransformComponent>());
            },
            "GetRigidBody", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasComponent<RigidBodyComponent>()) return sol::nil;
                auto& body = e.GetComponent<RigidBodyComponent>().RuntimeBody;
                if (!body) return sol::nil;
                return sol::make_object(s, body);
            },
            "GetLight", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasComponent<LightComponent>()) return sol::nil;
                return sol::make_object(s, &e.GetComponent<LightComponent>());
            },
            "GetCamera", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasComponent<SceneCameraComponent>()) return sol::nil;
                return sol::make_object(s, &e.GetComponent<SceneCameraComponent>());
            },
            "GetMaterial", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasComponent<MaterialComponent>()) return sol::nil;
                Shared<Material> material = AssetManager::GetAsset<Material>(e.GetComponent<MaterialComponent>().MaterialHandle);
                if (!material) return sol::nil;
                return sol::make_object(s, material);
            }
        );
    }

}
