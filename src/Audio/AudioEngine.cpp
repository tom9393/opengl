#include "AudioEngine.h"

#include <miniaudio/miniaudio.h>

#include "Asset/AssetManager.h"
#include "AudioClip.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

namespace Engine
{
    ma_engine* AudioEngine::s_Engine = nullptr;
    Scene* AudioEngine::s_Scene = nullptr;
    std::vector<AudioEngine::PlayingSound> AudioEngine::s_Sounds;

    void AudioEngine::Init()
    {
        s_Engine = new ma_engine();
        if (ma_engine_init(nullptr, s_Engine) != MA_SUCCESS)
        {
            std::cerr << "Failed to initialise audio engine" << std::endl;
            delete s_Engine;
            s_Engine = nullptr;
        }
    }

    void AudioEngine::Shutdown()
    {
        if (s_Engine)
        {
            ma_engine_uninit(s_Engine);
            delete s_Engine;
            s_Engine = nullptr;
        }

        s_Scene = nullptr;
    }

    Shared<AudioInstance> AudioEngine::CreateInstance(UUID clipHandle, bool spatial, bool loop, float volume, float pitch, float minDistance, float maxDistance)
    {
        if (!s_Engine || clipHandle == 0) return nullptr;

        Shared<AudioClip> clip = AssetManager::GetAsset<AudioClip>(clipHandle);
        if (!clip || !clip->IsValid()) return nullptr;

        Shared<AudioInstance> instance = MakeShared<AudioInstance>(s_Engine, clip, spatial, loop);
        if (!instance->IsValid()) return nullptr;

        instance->SetVolume(volume);
        instance->SetPitch(pitch);
        if (spatial) instance->SetAttenuation(minDistance, maxDistance);

        return instance;
    }

    Shared<AudioInstance> AudioEngine::Play(Entity emitter, const std::string& clipName, const AudioPlayParams& params)
    {
        if (!s_Engine) return nullptr;

        UUID clipHandle = AssetManager::GetHandleByName(clipName, AssetType::Audio);
        if (clipHandle == 0)
        {
            std::cerr << "Audio.Play: no audio clip named: " << clipName << std::endl;
            return nullptr;
        }

        Shared<AudioInstance> instance = CreateInstance(clipHandle, params.Spatial, params.Loop, params.Volume, params.Pitch, params.MinDistance, params.MaxDistance);
        if (!instance) return nullptr;

        if (params.Spatial && emitter.HasComponent<TransformComponent>())
            instance->SetPosition(glm::vec3(emitter.GetComponent<TransformComponent>().WorldTransform[3]));

        instance->Play();

        // Engine owns the sound and keeps it alive until it finishes
        // Finished sounds are discarded via OnUpdate once per game loop
        s_Sounds.push_back({ instance, emitter.GetHandle(), params.Spatial });
        return instance;
    }

    void AudioEngine::RuntimeStart(Scene* scene)
    {
        if (!s_Engine) return;
        s_Scene = scene;
    }

    void AudioEngine::RuntimeStop()
    {
        s_Sounds.clear();
        s_Scene = nullptr;
    }

    void AudioEngine::OnUpdate(TimeStep ts)
    {
        if (!s_Engine || !s_Scene) return;

        // Drive the listener from the primary scene camera.
        auto cameras = s_Scene->GetRegistry().view<SceneCameraComponent, TransformComponent>();
        for (auto e : cameras)
        {
            auto& camera = cameras.get<SceneCameraComponent>(e);
            if (!camera.primary) continue;

            const glm::mat4& world = cameras.get<TransformComponent>(e).WorldTransform;
            glm::vec3 position = glm::vec3(world[3]);
            glm::vec3 forward = glm::normalize(-glm::vec3(world[2]));
            glm::vec3 up = glm::normalize(glm::vec3(world[1]));

            ma_engine_listener_set_position(s_Engine, 0, position.x, position.y, position.z);
            ma_engine_listener_set_direction(s_Engine, 0, forward.x, forward.y, forward.z);
            ma_engine_listener_set_world_up(s_Engine, 0, up.x, up.y, up.z);
            break;
        }

        // Drop finished/stopped sounds, then keep spatial ones positioned at their emitting entity
        // Skip those whose emitter no longer exists
        s_Sounds.erase(std::remove_if(s_Sounds.begin(), s_Sounds.end(),
            [](const PlayingSound& sound) { return !sound.Instance || !sound.Instance->IsPlaying(); }),
            s_Sounds.end());

        entt::registry& registry = s_Scene->GetRegistry();
        for (auto& sound : s_Sounds)
        {
            if (!sound.Spatial) continue;
            if (!registry.valid(sound.Emitter) || !registry.all_of<TransformComponent>(sound.Emitter)) continue;

            glm::vec3 position = glm::vec3(registry.get<TransformComponent>(sound.Emitter).WorldTransform[3]);
            sound.Instance->SetPosition(position);
        }
    }
}
