#include "AudioBindings.h"

#include "pch.h"
#include "Audio/AudioEngine.h"
#include "Audio/AudioInstance.h"
#include "Scene/Entity.h"

namespace Engine {

    void AudioBindings::Register(sol::state& state)
    {
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
