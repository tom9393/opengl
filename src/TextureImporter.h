#pragma once

#include "Texture.h"
namespace Engine {

    class TextureImporter
    {
        public:
        static Shared<Texture2D> LoadTexture2D(const std::filesystem::path &path);
    };
}
