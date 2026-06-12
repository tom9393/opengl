#include "AudioImporter.h"

#include <miniaudio/miniaudio.h>

#include "AssetManager.h"
#include "AssetUploadQueue.h"
#include "Audio/AudioClip.h"
#include "Core/Application.h"

namespace Engine {

    Shared<AssetMetadata> AudioImporter::ImportAudio(const std::filesystem::path &path)
    {
        Shared<AssetMetadata> metadata = MakeShared<AssetMetadata>();
        metadata->Handle = UUID();
        metadata->FilePath = path.string();
        metadata->Name = path.stem().string();
        metadata->Type = AssetType::Audio;

        return metadata;
    }

    Shared<AudioClip> AudioImporter::Decode(const std::string &path)
    {
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_decoder decoder;
        if (ma_decoder_init_file(path.c_str(), &config, &decoder) != MA_SUCCESS) {
            std::cerr << "Failed to load audio: " << path << std::endl;
            return nullptr;
        }

        ma_uint64 frameCount = 0;
        ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);

        Shared<AudioClip> clip = MakeShared<AudioClip>();
        clip->m_Format = decoder.outputFormat;
        clip->m_Channels = decoder.outputChannels;
        clip->m_SampleRate = decoder.outputSampleRate;
        clip->m_FrameCount = frameCount;
        clip->m_Data.resize(frameCount * decoder.outputChannels * ma_get_bytes_per_sample(decoder.outputFormat));

        ma_uint64 framesRead = 0;
        ma_decoder_read_pcm_frames(&decoder, clip->m_Data.data(), frameCount, &framesRead);
        ma_decoder_uninit(&decoder);

        return clip;
    }

    Shared<Asset> AudioImporter::LoadAudio(const AssetMetadata &metadata)
    {
        return Decode(metadata.FilePath);
    }

    void AudioImporter::LoadAudioAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb)
    {
        PROFILE_FUNCTION();
        Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
            Shared<AudioClip> clip = Decode(metadata.FilePath);
            if (!clip) return;

            AssetUploadQueue::Submit([metadata, clip, cb]() {
                Shared<Asset> cast = clip;
                AssetManager::RegisterAsset(metadata, cast);
                if (cb) cb(clip);
            });
        });
    }
}
