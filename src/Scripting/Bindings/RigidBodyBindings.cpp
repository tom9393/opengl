#include "RigidBodyBindings.h"

#include "pch.h"
#include "Physics/RigidBody.h"

namespace Engine {

    void RigidBodyBindings::Register(sol::state& state)
    {
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
    }

}
