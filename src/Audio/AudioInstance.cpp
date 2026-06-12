#include "AudioInstance.h"

#include <miniaudio/miniaudio.h>

#include "AudioClip.h"

namespace Engine
{
    AudioInstance::AudioInstance(ma_engine* engine, Shared<AudioClip> clip, bool spatial, bool loop)
        : m_Clip(clip)
    {
        if (!clip || !clip->IsValid()) return;

        // Each instance gets its own cursor over the clip's shared PCM
        ma_audio_buffer_ref* bufferRef = new ma_audio_buffer_ref();
        m_BufferRef = bufferRef;
        ma_result result = ma_audio_buffer_ref_init((ma_format)clip->Format(), clip->Channels(), clip->Data().data(), clip->FrameCount(), bufferRef);
        if (result != MA_SUCCESS)
        {
            std::cerr << "Failed to create audio buffer ref" << std::endl;
            delete bufferRef;
            m_BufferRef = nullptr;
            return;
        }

        bufferRef->sampleRate = clip->SampleRate();

        m_Sound = new ma_sound();

        ma_uint32 flags = 0;
        if (!spatial) flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;

        result = ma_sound_init_from_data_source(engine, bufferRef, flags, nullptr, m_Sound);
        if (result != MA_SUCCESS)
        {
            std::cerr << "Failed to create audio sound" << std::endl;
            delete m_Sound;
            m_Sound = nullptr;
            ma_audio_buffer_ref_uninit(bufferRef);
            delete bufferRef;
            m_BufferRef = nullptr;
            return;
        }

        m_Valid = true;
        ma_sound_set_looping(m_Sound, loop ? MA_TRUE : MA_FALSE);
    }

    AudioInstance::~AudioInstance()
    {
        if (m_Sound)
        {
            ma_sound_uninit(m_Sound);
            delete m_Sound;
        }
        if (m_BufferRef)
        {
            ma_audio_buffer_ref* bufferRef = (ma_audio_buffer_ref*)m_BufferRef;
            ma_audio_buffer_ref_uninit(bufferRef);
            delete bufferRef;
        }
    }

    void AudioInstance::Play()
    {
        if (!m_Valid) return;
        ma_sound_seek_to_pcm_frame(m_Sound, 0);
        ma_sound_start(m_Sound);
    }

    void AudioInstance::Stop()
    {
        if (!m_Valid) return;
        ma_sound_stop(m_Sound);
    }

    bool AudioInstance::IsPlaying() const
    {
        if (!m_Valid) return false;
        return ma_sound_is_playing(m_Sound) == MA_TRUE;
    }

    void AudioInstance::SetVolume(float volume)
    {
        if (!m_Valid) return;
        ma_sound_set_volume(m_Sound, volume);
    }

    void AudioInstance::SetPitch(float pitch)
    {
        if (!m_Valid) return;
        ma_sound_set_pitch(m_Sound, pitch);
    }

    void AudioInstance::SetLooping(bool loop)
    {
        if (!m_Valid) return;
        ma_sound_set_looping(m_Sound, loop ? MA_TRUE : MA_FALSE);
    }

    void AudioInstance::SetSpatial(bool spatial)
    {
        if (!m_Valid) return;
        ma_sound_set_spatialization_enabled(m_Sound, spatial ? MA_TRUE : MA_FALSE);
    }

    void AudioInstance::SetAttenuation(float minDistance, float maxDistance)
    {
        if (!m_Valid) return;
        ma_sound_set_min_distance(m_Sound, minDistance);
        ma_sound_set_max_distance(m_Sound, maxDistance);
    }

    void AudioInstance::SetPosition(const glm::vec3& position)
    {
        if (!m_Valid) return;
        ma_sound_set_position(m_Sound, position.x, position.y, position.z);
    }

    void AudioInstance::SetDirection(const glm::vec3& direction)
    {
        if (!m_Valid) return;
        ma_sound_set_direction(m_Sound, direction.x, direction.y, direction.z);
    }
}
