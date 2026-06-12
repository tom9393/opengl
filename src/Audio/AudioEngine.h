#pragma once

#include "pch.h"
#include "AudioInstance.h"
#include "Core/TimeStep.h"
#include "Core/UUID.h"

struct ma_engine;

namespace Engine
{
    class Scene;
    class Entity;

    // Parameters exposed by Lua
    struct AudioPlayParams
    {
        float Volume = 1.0f;
        float Pitch = 1.0f;
        bool Loop = false;
        bool Spatial = true;        // false -> non-spatial 2D i.e. menues
        float MinDistance = 1.0f;   // Only used with spatial audio
        float MaxDistance = 50.0f;
    };

    class AudioEngine
    {
        public:
        static void Init();
        static void Shutdown();

        static void OnUpdate(TimeStep ts);

        static void RuntimeStart(Scene* scene);
        static void RuntimeStop();

        // Looks up an audio clip by asset name and plays it
        // The engine owns the sound, returns instance so scripts can stop/adjust them
        static Shared<AudioInstance> Play(Entity emitter, const std::string& clipName, const AudioPlayParams& params);

        // Builds a runtime AudioInstance for a given clip
        static Shared<AudioInstance> CreateInstance(UUID clipHandle, bool spatial, bool loop, float volume, float pitch, float minDistance, float maxDistance);

        static ma_engine* GetEngine() { return s_Engine; }

        private:
        // Sound owned by engine and destroyed upon finishing
        struct PlayingSound
        {
            Shared<AudioInstance> Instance;
            entt::entity Emitter = entt::null;
            bool Spatial = false;
        };

        static ma_engine* s_Engine;
        static Scene* s_Scene;
        static std::vector<PlayingSound> s_Sounds;
    };
}
