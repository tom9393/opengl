#include "ParticleBindings.h"

#include "pch.h"
#include "Scene/Components.h"
#include "Scene/ParticleSystem.h"

namespace Engine {

    void ParticleBindings::Register(sol::state& state)
    {
        // Play/Stop/Emit forward to the runtime system; it is created by
        // ParticleEngine on the first update after play starts.
        state.new_usertype<ParticleEmitterComponent>("ParticleEmitterComponent",
            "Rate", &ParticleEmitterComponent::Rate,
            "Looping", &ParticleEmitterComponent::Looping,
            "Play", [](ParticleEmitterComponent& pe) { if (pe.Runtime) pe.Runtime->Play(); },
            "Stop", [](ParticleEmitterComponent& pe) { if (pe.Runtime) pe.Runtime->Stop(); },
            "Emit", [](ParticleEmitterComponent& pe, uint32_t count) { if (pe.Runtime) pe.Runtime->Emit(count); }
        );
    }

}
