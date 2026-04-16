#pragma once

#include "Texture.h"
namespace Engine {

    class TextureImporter
    {
        public:
        static Shared<Texture2D> LoadTexture2D(Shared<AssetMetadata> &metadata);

        static Shared<AssetMetadata> ImportTexture2D(const std::filesystem::path &path);
    };
}
