#include "TransformBindings.h"

#include "pch.h"
#include "Scene/Components.h"
#include "Utils/MathTools.h"

namespace Engine {

    void TransformBindings::Register(sol::state& state)
    {
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
    }

}
