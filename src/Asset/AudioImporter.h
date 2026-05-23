#pragma once

#include "pch.h"
#include "Asset.h"

namespace Engine {

    class AudioClip;

    class AudioImporter
    {
        public:
        static Shared<AssetMetadata> ImportAudio(const std::filesystem::path &path);

        static Shared<Asset> LoadAudio(const AssetMetadata &metadata);
        static void LoadAudioAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb);

        private:
        static Shared<AudioClip> Decode(const std::string &path);
    };
}
