#pragma once

#include "Texture.h"

namespace Engine {

    struct TextureLoadData {
        unsigned char* Pixels = nullptr;
        // When set, the texture is created as a float (16F) image from this data
        // instead of from Pixels. Used for HDR sources such as equirectangular maps.
        float* HDRPixels = nullptr;
        int Width = 0, Height = 0, Channels = 0;
        bool clamp = false;
    };

    class TextureImporter
    {
        public:
        static Shared<Texture2D> LoadTexture2D(const AssetMetadata &metadata);
        static void LoadTexture2DAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb);

        static Shared<Texture2D> LoadHDR(const std::string& path);

        static Shared<AssetMetadata> ImportTexture2D(const std::filesystem::path &path);

        static TextureLoadData DecodeTextureFromMemory(const void* data, size_t length);
        static TextureLoadData DecodeTextureUncompressed(const void* data, uint32_t width, uint32_t height);
        static void FreeTextureData(const TextureLoadData& data);
    };
}
