#pragma once

#include "pch.h"

struct ma_engine;
struct ma_sound;

namespace Engine
{
    class AudioClip;

    // RAII wrapper around a single miniaudio ma_sound. This is the runtime handle,
    // owned by the AudioEngine while it plays, analogous to a RigidBody's RuntimeBody.
    // The sound reads from a per-instance cursor over the clip's shared PCM buffer.
    class AudioInstance
    {
        public:
        AudioInstance(ma_engine* engine, Shared<AudioClip> clip, bool spatial, bool loop);
        ~AudioInstance();

        AudioInstance(const AudioInstance&) = delete;
        AudioInstance& operator=(const AudioInstance&) = delete;

        void Play(); // Restarts from the beginning
        void Stop();

        bool IsPlaying() const;
        bool IsValid() const { return m_Valid; }

        void SetVolume(float volume);
        void SetPitch(float pitch);
        void SetLooping(bool loop);
        void SetSpatial(bool spatial);
        void SetAttenuation(float minDistance, float maxDistance);
        void SetPosition(const glm::vec3& position);
        void SetDirection(const glm::vec3& direction);

        private:
        ma_sound* m_Sound = nullptr;
        void* m_BufferRef = nullptr; // ma_audio_buffer_ref*
        Shared<AudioClip> m_Clip; // Keeps the PCM the buffer ref points at alive.
        bool m_Valid = false;
    };
}
