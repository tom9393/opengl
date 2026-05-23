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

    // Per-call options for Audio.Play. Mirrors the table fields exposed to Lua;
    // any field omitted by the script falls back to these defaults.
    struct AudioPlayParams
    {
        float Volume = 1.0f;
        float Pitch = 1.0f;
        bool Loop = false;
        bool Spatial = true;       // false => non-spatial 2D (music / UI)
        float MinDistance = 1.0f;  // attenuation, only used when Spatial
        float MaxDistance = 50.0f;
    };

    // Static singleton wrapping miniaudio's ma_engine (device + node graph +
    // resource manager + 3D spatializer). Mirrors PhysicsEngine / ScriptEngine.
    class AudioEngine
    {
        public:
        static void Init();
        static void Shutdown();

        static void OnUpdate(TimeStep ts);

        static void RuntimeStart(Scene* scene);
        static void RuntimeStop();

        // Looks up an audio clip by asset name and plays it. The engine owns the
        // live sound; for a spatial sound it follows `emitter`'s transform each
        // frame (and reaps the sound once it finishes). Returns the instance so
        // scripts can stop/adjust it; nullptr if the clip is missing.
        static Shared<AudioInstance> Play(Entity emitter, const std::string& clipName, const AudioPlayParams& params);

        // Builds a runtime AudioInstance for a clip from the given properties.
        static Shared<AudioInstance> CreateInstance(UUID clipHandle, bool spatial, bool loop, float volume, float pitch, float minDistance, float maxDistance);

        static ma_engine* GetEngine() { return s_Engine; }

        private:
        // A sound currently owned by the engine. Spatial sounds track their
        // emitter entity's transform each frame; all are reaped when finished.
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
