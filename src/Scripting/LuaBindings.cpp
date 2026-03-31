#include "LuaBindings.h"
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
            sol::meta_function::multiplication, [](const glm::vec3& a, float b) { return a * b; }
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

    void LuaBindings::RegisterComponents(sol::state& state) {
        // 1. Transform Component
        state.new_usertype<TransformComponent>("TransformComponent",
            "Translation", &TransformComponent::translation,
            "Rotation", &TransformComponent::rotation,
            "Scale", &TransformComponent::scale,
            "GetForward", &TransformComponent::GetForward,
            "GetRight", &TransformComponent::GetRight
        );

        // 2. RigidBody Component (Assuming you want to apply forces via script)
        // Adjust the function pointers based on your actual RigidBody.h implementation
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

        // 3. Camera Component
        state.new_usertype<SceneCameraComponent>("CameraComponent",
            "Primary", &SceneCameraComponent::primary,
            "FixedAspectRatio", &SceneCameraComponent::fixedAspectRatio
        );
    }

}