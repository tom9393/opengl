#include "LightBindings.h"

#include "pch.h"
#include "Scene/Components.h"

namespace Engine {

    void LightBindings::Register(sol::state& state)
    {
        state.new_usertype<LightComponent>("LightComponent",
            "Color", &LightComponent::Color,
            "Intensity", &LightComponent::Intensity,
            "CastShadows", &LightComponent::CastShadows
        );
    }

}
