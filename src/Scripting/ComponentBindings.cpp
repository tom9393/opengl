#include "ComponentBindings.h"

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Asset/Prefab.h"
#include "Audio/AudioEngine.h"
#include "Audio/AudioInstance.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/RigidBody.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Utils/MathTools.h"

namespace Engine {

    void ComponentBindings::Register(sol::state& state)
    {

        state.new_usertype<RaycastResult>("RaycastResult",
            "Hit",      &RaycastResult::Hit,
            "Entity",   &RaycastResult::HitEntity,
            "Position", &RaycastResult::Position,
            "Normal",   &RaycastResult::Normal
        );

        state.new_usertype<Entity>("Entity",
            "Destroy", [](Entity& e) { e.GetScene()->DestroyEntity(e); },
            "GetLuaEnvironment", [](Entity& e) -> sol::object {
                if (e.HasComponent<LuaScriptComponent>()) {
                    auto& script = e.GetComponent<LuaScriptComponent>();

                    if (script.Environment.valid()) {
                        return script.Environment;
                    }
                }
                return sol::nil;
            },
            "GetParent", [](Entity& e, sol::this_state s) -> sol::object {
                if (!e.HasParent()) return sol::nil;
                return sol::make_object(s, Entity{ e.GetParent(), e.GetScene() });
            }
        );

        state.new_usertype<TransformComponent>("TransformComponent",
            "Translation", &TransformComponent::translation,
            "Rotation", &TransformComponent::rotation,
            "Scale", &TransformComponent::scale,
            "GetForward", &TransformComponent::GetForward,
            "GetRight", &TransformComponent::GetRight,
            "RotateTowards", [](TransformComponent &transformComponent, const glm::vec3 &from, const glm::vec3 &to) {
                return RotateTowards(from, to);
            }
        );

        state.new_usertype<Scene>("Scene",
        "CreateEntity", &Scene::CreateEntity,
        "Raycast", [](Scene& scene, const glm::vec3 origin, const glm::vec3 dir, float dist) {
            return PhysicsEngine::Raycast(origin, dir, dist);
        },
        // Scene.Instantiate("prefabName", position, rotation) -> Entity.
        // position/rotation are optional vec3s (rotation is euler radians).
        "Instantiate", [](Scene& scene, const std::string& name, sol::optional<glm::vec3> position, sol::optional<glm::vec3> rotation) -> Entity {
            UUID handle = AssetManager::GetHandleByName(name, AssetType::Prefab);
            if (handle == 0) {
                std::cerr << "Scene.Instantiate: no prefab named '" << name << "'" << std::endl;
                return Entity{};
            }
            Shared<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
            glm::vec3 p = position ? *position : glm::vec3(0.0f);
            glm::vec3 r = rotation ? *rotation : glm::vec3(0.0f);
            return scene.InstantiatePrefab(prefab, p, r);
        });

        state.new_usertype<TagComponent>("TagComponent",
            "Tag", &TagComponent::tag
        );

        state.new_usertype<RigidBody>("RigidBody",
            "SetTransform", &RigidBody::SetTransform,
            "GetTransform", &RigidBody::GetTransform,

            "GetForward", &RigidBody::GetForward,
            "GetUp", &RigidBody::GetUp,
            "GetRight", &RigidBody::GetRight,

            "LookAt", &RigidBody::LookAt,
            "MoveForward", &RigidBody::MoveForward,
            "ApplyRelativeImpulse", &RigidBody::ApplyRelativeImpulse,

            "ApplyForce", &RigidBody::ApplyForce,
            "ApplyImpulse", &RigidBody::ApplyImpulse,
            "ApplyTorque", &RigidBody::ApplyTorque,
            "ClearForces", &RigidBody::ClearForces,

            "SetLinearVelocity", &RigidBody::SetLinearVelocity,
            "GetLinearVelocity", &RigidBody::GetLinearVelocity,
            "SetAngularVelocity", &RigidBody::SetAngularVelocity,

            "SetMass", &RigidBody::SetMass,
            "SetDamping", &RigidBody::SetDamping,
            "SetRestitution", &RigidBody::SetRestitution,
            "SetFriction", &RigidBody::SetFriction,
            "SetGravity", &RigidBody::SetGravity,

            "SetLinearFactor", &RigidBody::SetLinearFactor,
            "SetAngularFactor", &RigidBody::SetAngularFactor,
            "SetKinematic", &RigidBody::SetKinematic,
            "SetTrigger", &RigidBody::SetTrigger
        );

        state.new_usertype<SceneCameraComponent>("CameraComponent",
            "Primary", &SceneCameraComponent::primary,
            "FixedAspectRatio", &SceneCameraComponent::fixedAspectRatio
        );

        // Handle returned by Audio.Play, so scripts can stop/adjust a live sound.
        state.new_usertype<AudioInstance>("AudioInstance",
            "Play", &AudioInstance::Play,
            "Stop", &AudioInstance::Stop,
            "IsPlaying", &AudioInstance::IsPlaying,
            "SetVolume", &AudioInstance::SetVolume,
            "SetPitch", &AudioInstance::SetPitch,
            "SetLooping", &AudioInstance::SetLooping
        );

        // Global audio API, available to every script (not tied to a component).
        // Audio.Play("clip", entity, { volume=, pitch=, loop=, spatial=, minDistance=, maxDistance= })
        // plays the named clip; for a spatial sound the engine follows `entity`'s
        // transform each frame. Returns the sound so the script can stop/adjust it.
        sol::table audio = state.create_table("Audio");
        audio.set_function("Play", [](const std::string& name, Entity emitter, sol::optional<sol::table> opts) -> Shared<AudioInstance> {
            AudioPlayParams params;
            if (opts)
            {
                sol::table t = *opts;
                params.Volume      = t.get_or("volume", params.Volume);
                params.Pitch       = t.get_or("pitch", params.Pitch);
                params.Loop        = t.get_or("loop", params.Loop);
                params.Spatial     = t.get_or("spatial", params.Spatial);
                params.MinDistance = t.get_or("minDistance", params.MinDistance);
                params.MaxDistance = t.get_or("maxDistance", params.MaxDistance);
            }
            return AudioEngine::Play(emitter, name, params);
        });
    }

}
